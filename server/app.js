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
  	
	if(clients.length == 2) {
		for(var i = 0; i < 2; i++ ) {
			var client = clients[i];
			var otherClient = clients[!i];
		
			client.oscOut.send('/setRemote', otherClient.ip, 7000+i*10);
			client.oscOut.send('/setPort', 7000+(!i)*10);   
		}
	} else {
		console.log("There needs to be exactly two clients");
	}
}

var reset = function() {
	remotes = [];
}

app.get('/', function(req, res){
  res.send('This is the server for establishing contact between the portals.');
});

app.get('/reset', function(req, res){
  reset();
  res.send('Remotes have been reset.');
});

oscIn.on("message", function (msg, rinfo) {
		
	var mI = 0; // messsage index offset for handling osc bundles 
	if(msg[0] == "#bundle") {
		mI = 2;
	}
		
	if(msg[mI] == "/hello") {
		var responsePort = msg[mI+1];
    console.log("Received hello from: " + rinfo.address + 
	  			  ". Responding on port " + responsePort);
	  
	  if(clients.length > 1) {	  	
			console.log("Already has two clients, call /reset to reconfigure.");
	  } else {
	  	var client = new Client(rinfo.address, responsePort);
	  	clients.push(client);
		
	  	if(clients.length == 2) {
				// We have 2 clients, lets set them up
		  	setupClients();
	  	}	
		}
  } else if (msg[0] == "/reset") {
  	reset();
  } else {
		console.log(" Jeg aner ikke hvad jeg skal med den besked. " + msg)
	}
	
	
	 
});

app.listen(3000);