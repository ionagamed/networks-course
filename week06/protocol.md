# node protocol

## first node

1. `./node initial --lport 2000`

## join

1. `./node join 192.168.1.2:2000 --lport 2000`
2. me -> him: ADD_NODE, me, ttl=4
3. him -> me: ADD_NODE_OK

## push to others

1. me -> him: JOIN, other
2. him -> me: JOIN_OK
3. him -> other: ADD_NODE, him
4. other -> him: ADD_NODE_OK
5. he pushes to others with decreased TTL