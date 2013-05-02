var express = require('express');
var _ = require('underscore');
var app = express();
var osc = require('osc-min');
var dgram = require('dgram')
var util = require("util");


var inPort = 8000;
var httpPort = 3000;

var oscServer = dgram.createSocket("udp4");
var oscClient = dgram.createSocket("udp4");

var Client = function(ip, port, index) {
	this.index = index;
	this.port = port;
	this.ip = ip;
	this.dataPort = 7000+index*10
	
	this.send = function(oscBuffer) {
		console.log("Send "+oscBuffer);
		oscClient.send(oscBuffer, 0, oscBuffer.length, this.port, this.ip);
	}
}

var clients = [];

var setupClients = function() {
  	
	if(clients.length == 2) {
		console.log("Setting up clients to talk with each other");
		
		for(var i = 0; i < 2; i++ ) {
			
			var client = clients[i];
			var otherClient = clients[1-i];
			console.log("Telling client "+i+" to send to "+otherClient.ip+":"+otherClient.dataPort+"  and listen on "+client.dataPort);
						
			client.send(osc.toBuffer("/setRemote", [otherClient.ip, otherClient.dataPort]));
			client.send(osc.toBuffer("/setPort", client.dataPort));
		}
		
	} else {
		console.log("There needs to be exactly two clients");
	}
}

var reset = function() {
	remotes = [];
}

var parseOsc =  function (msg, rinfo) {
	
	console.log("parsing message: " + util.inspect(msg));
	
	//If its a bundle, read the first message of the bundle
	if(msg.oscType == "bundle"){
		msg = msg.elements[0];
	}
	
	if(msg.address == "/hello") {
		
		var responsePort = msg.args[0].value;
    console.log("Received hello from: " + rinfo.address + 
	  			  ". Responding on port " + responsePort);
	  
	  if(clients.length > 1) {	  	
			console.log("Already has two clients. Resetting...");
			reset();
	  }
			
	  var client = new Client(rinfo.address, responsePort, clients.length);
	  clients.push(client);
		
	  if(clients.length == 2) {
			// We have 2 clients, lets set them up
		 	setupClients();
	  }	
	
	} else if (msg.address == "/reset") {
  	reset();
  } else {
		console.log("Unknown message: " + msg)
	}	 
}

oscServer.on("message", function (msg, rinfo) { 
	
	parseOsc(osc.fromBuffer(msg), rinfo);
		
});

app.get('/', function(req, res){
  res.render("index.html");
});

app.get('/reset', function(req, res){
  reset();
  res.send('Remotes have been reset.');
});

oscServer.bind(inPort);
app.listen(httpPort);