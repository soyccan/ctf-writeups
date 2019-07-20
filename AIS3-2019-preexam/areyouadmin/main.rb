#!/usr/bin/ruby
require 'json'

STDOUT.sync = true

puts "Your name:"
name = STDIN.gets.chomp
puts "Your age:"
age = STDIN.gets.chomp

if age.match(/[[:alpha:]]/)
    puts "No!No!No!"
    exit
end

# {"name": "     ","is_admin":"yes",/*"      ", "is_admin":"no", "age": "       */"":"        "}
# {"name": "", "is_admin":"no", "age": "       3","is_admin":"yes","":"        "}
# 3","is_admin":"yes","":"
# 3","\108\115_\97\100\109\105\110":"\121\101\115","":"
# 3","\u108\u115_\u97\u100\u109\u105\u110":"\u121\u101\u115","":"

string = "{\"name\":\"#{name}\",\"is_admin\":\"no\", \"age\":\"#{age}\"}"
res = JSON.parse(string)

if res['is_admin'] == "yes"
    puts "AIS3{xxxxxxxxxxxx}"  # flag is here
else
    puts "Hello, " + res['name']
    puts "You are not admin :("
end
