var express = require('express');
var _ = require('underscore');
var app = express();
var osc = require('node-osc');

//var Remote = require("../remote");
var oscIn = new osc.Server(8000, '0.0.0.0');

var Client = function(ip, port) {
	
	this.oscOut= new osc.Client(ip, port);
	
	this.port = port;
	this.ip = ip;
}

var clients = [];

var setupClients = function() {
  	
	for(var i = 0; i < 2; i++ ) {
		var client = clients[i];
		var otherClient = clients[!i];
		
		client.oscOut.send('/setRemote', otherClient.ip, 7000+i*10);
		client.oscOut.send('/setPort', 7000+(!i)*10);   
	}
	
}

app.get('/', function(req, res){
  res.send('This is the server for establishing contact between the portals.');
});

app.get('/reset', function(req, res){
  remotes = [];
  res.send('Remotes have been reset.');
});

oscIn.on("/hello", function (msg, rinfo) {
	  
	  var responsePort = msg[1];
      console.log("Received hello from: " + rinfo.address + 
	  			  ". Responding on port " + responsePort);
	  
	  var client = new Client(rinfo.address, responsePort);
	  
	  clients.push(client);
	  
	  if(clients.length > 1) {
		  // We have 2 clients
		  setupClients();
	  }
	  
});


app.listen(3000);