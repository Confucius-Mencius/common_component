print(package.path)
print('**')
print(package.cpath)

--package.path = package.path ..';../?.lua'
package.cpath = package.cpath .. ';../?.so'

--local libc_api_swig = require('libc_api_swig') -- 这样加载会默认生成一层名为libc_api_swig的table，所有的符号都在这个table中
--package.loadlib("libc_api_swig.so","SWIG_init") -- 这样加载不会生成一层table

local c_module1 = require('libc_api_swig_module1')
local c_module2 = require('libc_api_swig_module2')

-- Call our Demo1Func() function
x = 42
y = 105
g = c_module1.SomeFunc(x, y)
print("The SomeFunc of", x, "and", y, "is", g)

print('-------------')
-- Manipulate the Foo global variable
-- Output its current value
print("g_foo = ", c_module1.g_foo)

-- Change its value
c_module1.g_foo = 3.1415926

-- See if the change took effect
print("g_foo = ", c_module1.g_foo)

print('-------------')
print(c_module2.SomeFunc1())
print(c_module2.SomeFunc2('wow'))

print('-------------')
print(c_module1.MAX_X)
print(c_module1.TYPE_1)
print(c_module1.TYPE_2)
