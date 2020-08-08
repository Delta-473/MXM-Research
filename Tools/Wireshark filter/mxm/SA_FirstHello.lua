local helpers = require("helpers")

local m = {
	dwProtocolCRC = ProtoField.uint32("mxm.dwProtocolCRC", "dwProtocolCRC", base.HEX),
	dwErrorCRC = ProtoField.uint32("mxm.dwErrorCRC", "dwErrorCRC", base.HEX),
	serverType = ProtoField.uint8("mxm.serverType", "serverType", base.HEX),
	clientIp = ProtoField.string("mxm.clientIp", "clientIp", base.ASCII),
	clientPort = ProtoField.uint16("mxm.clientPort", "clientPort", base.DEC),
	tqosWorldId = ProtoField.uint8("mxm.tqosWorldId", "tqosWorldId", base.HEX)
}

function m.get_fields()
	local fields = {
		dwProtocolCRC = m.dwProtocolCRC,
		dwErrorCRC = m.dwErrorCRC,
		serverType = m.serverType,
		clientIp = m.clientIp,
		clientPort = m.clientPort,
		tqosWorldId = m.tqosWorldId
	}
	
	return fields
end

function m.parse(payloadSubtree, buffer, length)
	payloadSubtree:add_le(m.dwProtocolCRC, buffer(4, 4))
	payloadSubtree:add_le(m.dwErrorCRC, buffer(8,4))
	payloadSubtree:add_le(m.serverType, buffer(12,1))
	payloadSubtree:add_le(m.clientIp, helpers.convertIpToString(buffer(13,4)))
	payloadSubtree:add_le(m.clientPort, buffer(17,2))
	payloadSubtree:add_le(m.tqosWorldId, buffer(19,1))
end

return m