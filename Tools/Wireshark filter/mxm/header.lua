function get_opcode_name(opcode)
	local opcode_name = "Unknown"

	--client
		if opcode == 60002 then opcode_name = "Cl_Hello" 
	elseif opcode == 60003 then opcode_name = "Cl_Login"
	elseif opcode == 60005 then opcode_name = "Cl_ConfirmGatewayInfo"
	elseif opcode == 60007 then opcode_name = "Cl_enterQueue"
	elseif opcode == 60008 then opcode_name = "Cl_RequestConnectGame"
	elseif opcode == 60035 then opcode_name = "Cl_ConfirmLogin"
	elseif opcode == 60049 then opcode_name = "CQ_SET_NICKNAME"
	elseif opcode == 60050 then opcode_name = "CQ_CHECK_DUP_NICKNAME"
	--server
	elseif opcode == 62002 then opcode_name = "SA_FirstHello"
	elseif opcode == 62003 then opcode_name = "SA_UserloginResult"
	elseif opcode == 62005 then opcode_name = "SA_AuthResult"
	elseif opcode == 62006 then opcode_name = "SN_RegionServicePolicy"
	elseif opcode == 62009 then opcode_name = "SN_TgchatServerInfo"
	elseif opcode == 62047 then opcode_name = "SA_VersionInfo"
	elseif opcode == 62501 then opcode_name = "Sv_QueueStatus"
	elseif opcode == 62011 then opcode_name = "Sv_Finish"
		end --of "switch" case

	return opcode_name
end

local m = {
	packet_length = ProtoField.uint16("mxm.packet_length", "Packet Length", base.DEC),
	packet_id = ProtoField.uint16("mxm.packet_id", "Packet ID", base.DEC)
}

function m.get_fields()
	local fields = {
		packet_length = m.packet_length,
		packet_id = m.packet_id
	}
	
	return fields
end

function m.parse(headerSubtree, buffer)
	headerSubtree:add_le(m.packet_length, buffer(0,2))

	local opcode = buffer(2,2):le_uint()
	local opcode_name = get_opcode_name(opcode)
	headerSubtree:add_le(m.packet_id, buffer(2,2)):append_text("(".. opcode_name .. ")")

	return opcode_name
end

return m