function Add(a, b)
    return (a + b)
end

function OKFunc(a, b, c)
    return a, b, c
end

function ErrFunc(a, b)
    NotExistFunc()
end

print(Add(1, 2))
a, b, c = OKFunc(1, "!", 2.0)
print('in lua: ' .. a .. ' ' .. b .. ' ' .. c)

a, b, c = OKFunc(4294967296, "!", 34000000000000000000000000000000000000000.0)
print('in lua: ' .. a .. ' ' .. b .. ' ' .. c)

-- toluapp api
-- dofile("data/toluapp_test.lua")

-- swig_api
-- dofile("data/swig_test.lua")
