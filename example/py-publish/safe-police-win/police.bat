echo "deny 42,88,135,137,139,445 port"
netsh advfirewall firewall add rule name="RPC_Port_135-137-139-445_deny" dir=in  protocol=TCP localport=42,88,135,137,139,445  action=block
netsh advfirewall firewall add rule name="RPC_Port_135-137-139-445_deny" dir=in  protocol=UDP localport=88,135,137,139,445  action=block
exit 0