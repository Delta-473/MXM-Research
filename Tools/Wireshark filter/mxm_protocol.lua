--MXM protocol analyzer

package.prepend_path("plugins/mxm")
package.prepend_path("plugins/mxm/server")
package.prepend_path("plugins/mxm/client")
local header = require("header")
local helpers = require("helpers")
local sa_firstHallo = require("SA_FirstHello")
local sa_userloginResult = require("SA_UserloginResult")
local sa_authResult = require("SA_AuthResult")

mxm_proto = Proto("mxm", "Master X Master Protocol")

--Default settings
local default_settings = {
	auth_server_port = 10900,
	game_server_port = 11900
}

--Fields
local header_fields = header.get_fields()
local sa_firstHallo_fields = sa_firstHallo.get_fields()
local sa_userloginResult_fields = sa_userloginResult.get_fields()
local sa_authResult_fields = sa_authResult.get_fields()

	--add the fields to the dissector
helpers.merge_tables(header_fields, mxm_proto.fields)
helpers.merge_tables(sa_firstHallo_fields, mxm_proto.fields)
helpers.merge_tables(sa_userloginResult_fields, mxm_proto.fields)
helpers.merge_tables(sa_authResult_fields, mxm_proto.fields)

-- Main function
function mxm_proto.dissector(buffer, pinfo, tree)
	length = buffer:len()
	if lenght == 0 then return end

	pinfo.cols.protocol = mxm_proto.name

	local subtree = tree:add(mxm_proto, buffer(), "MXM Protocol Data")
	local headerSubtree = subtree:add(mxm_proto, buffer(),"Header")
	local payloadSubtree = subtree:add(mxm_proto, buffer(), "Payload")

	local opcode_name = header.parse(headerSubtree, buffer)

	--Payload
	print(opcode_name)
	if opcode_name == "SA_FirstHello" then sa_firstHallo.parse(payloadSubtree, buffer, lenght)
	elseif opcode_name == "SA_UserloginResult" then sa_userloginResult.parse(payloadSubtree, buffer, lenght)
	elseif opcode_name == "SA_AuthResult" then sa_authResult.parse(payloadSubtree, buffer, lenght)
	end

end

--Settings Menu
	--register the preferences
mxm_proto.prefs.authserver =  Pref.uint("Auth server port", default_settings.auth_server_port, "The server port for the auth server")
mxm_proto.prefs.gameserver = Pref.uint("Game server port", default_settings.game_server_port, "The server port for the game server")
	--handeling preferences changes
function mxm_proto.prefs_changed()
	--ToDO change the ports
	reload()
end



local tcp_port = DissectorTable.get("tcp.port")
tcp_port:add(default_settings.auth_server_port, mxm_proto)
tcp_port:add(default_settings.game_server_port, mxm_proto)
