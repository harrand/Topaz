tz.gl.find_renderer_by_name = function(name)
	local rc = tz.gl.get_device():renderer_count()
	if rc <= 0 then return nil end
	for i=0, rc-1, 1 do
		local ren = tz.gl.get_device():get_renderer(i)
		if ren:debug_get_name() == name then
			return ren
		end
	end
	return nil
end