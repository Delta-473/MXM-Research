local m = {
    result = ProtoField.int32("mxm.result", "result", base.DEC)
}

function m.get_fields()
    local fields = {
        result = m.result
    }

    return fields
end

function m.parse(payloadSubtree, buffer, length)
    payloadSubtree:add_le(m.result, buffer(4,4))
end

return m