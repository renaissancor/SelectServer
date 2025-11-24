# Star Server 

Divide Session and Gameplay. 
How? 

L5 Session 
L7 Gameplay 

## Game Server Logic (Single Thread)

Receive Packet and process directly 
Send packet that require immediate update 
Server Logic 
Send packet that require logic 
Display Viewer (if activated)
Rest to maintain tick time 

Session 

If Listen Socket receive new connection 
Create new session 
Create new player in L7 

If Session receive packet
Process packet
If received packet returns WOULD_BLOCK 
client did not send packet completely

Whenever send packet returns WOULD_BLOCK
send fail means client is in abnormal state 
close Socket and disconnect and remove player. 

