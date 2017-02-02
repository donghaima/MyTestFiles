import xmlrpclib

# Create an object to represent our server.
server_url = 'http://127.0.0.1:8051/RPC2';
server = xmlrpclib.Server(server_url);

# Call the server and get our result.
result = server.sample.sumAndDifference(5, 3)
print "Sum:", result['sum']
print "Difference:", result['difference']
