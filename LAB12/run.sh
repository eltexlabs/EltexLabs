# Get IP from user
echo "Your IPs:"
echo `hostname -I`
read -p "Enter ip address: " var_ip

# Launch clients of type 1
xterm -geometry 50x15+0+0	-e "./cl1.elf $var_ip 2000 2001" &
xterm -geometry 50x15+0+260	-e "./cl1.elf $var_ip 2000 2001" &
xterm -geometry 50x15+0+490	-e "./cl1.elf $var_ip 2000 2001" &

# Launch clients of type 2
xterm -geometry 50x15+700+0	-e "./cl2.elf $var_ip 2000 2001" &
xterm -geometry 50x15+700+260	-e "./cl2.elf $var_ip 2000 2001" &
xterm -geometry 50x15+700+490	-e "./cl2.elf $var_ip 2000 2001" &

# Launch server
xterm -geometry 50x50+350+0	-e "./serv.elf 2000 2001 10" &
