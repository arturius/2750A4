'use strict'

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
const mysql = require('mysql');
// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }


  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 
const calLib = ffi.Library('./libcal', {
    'calJSONFromFileName': [ 'string', ['string'] ],
    'eventListJSONFromFileName': [ 'string', ['string'] ],
    'propListJSONFromFileName': [ 'string', ['string','string'] ],
    'alarmListJSONFromFileName': [ 'string', ['string','string'] ],

    'createCalFileFromJSON': [ 'string', ['string','string','string'] ],
    'addEventToCalFileFromJSON': [ 'string', ['string','string'] ],
});

let connection;

app.get('/databaseValues', function(req , res){
    connection = mysql.createConnection({
        host     : req.query.host,
        user     : req.query.user,
        password : req.query.password,
        database : req.query.database
    });
    connection.connect(function(err) {
        if (err){ 
            throw err;
        }
        console.log("Connected!");
    });
    let fileTable = true;
    let eventTable = true;
    let alarmTable = true;
    connection.query("show tables", function (err, rows, fields) {
        if (err){
            console.log("Something went wrong. "+err)
    	}else{
		    for (let row of rows){
                console.log(row.Tables_in_jander21+""+fileTable+""+eventTable+""+alarmTable);
                switch(row.Tables_in_jander21){
                    case "FILE":
                        fileTable = false;
                        break;
                    case "EVENT":
                        eventTable = false;
                        break;
                    case "ALARM":
                        alarmTable = false;
                        break;
                }
            }
	    }
    });

    if (fileTable===true){
        console.log("creating file Table in database");
        connection.query("create table FILE ("+
            "cal_id INT AUTO_INCREMENT PRIMARY KEY,"+
            "file_Name VARCHAR(60) NOT NULL,"+
            "version INT NOT NULL,"+
            "prod_id VARCHAR(256) NOT NULL)",
            function (err, rows, fields) {
            if (err) console.log(err);
        });
    }

    if (eventTable===true){
        console.log("creating event Table in database");
        connection.query("create table EVENT ("+
            "event_id INT AUTO_INCREMENT PRIMARY KEY,"+
            "summary VARCHAR(1024),"+
            "start_time DATETIME NOT NULL,"+
            "location VARCHAR(60),"+
            "organizer VARCHAR(256),"+
            "cal_file INT NOT NULL,"+
            "FOREIGN KEY (cal_file) REFERENCES FILE(cal_id) ON DELETE CASCADE)",
            function (err, rows, fields) {
            if (err) console.log(+err);
        });
    }
    if (alarmTable=== true){
        console.log("creating alarm Table in database");
        connection.query("create table ALARM ("+
            "alarm_id INT AUTO_INCREMENT PRIMARY KEY,"+
            "action VARCHAR(256) NOT NULL, "+
            "`trigger` VARCHAR(256) NOT NULL,"+
            "event INT NOT NULL,"+
            "FOREIGN KEY (event) REFERENCES EVENT(event_id) ON DELETE CASCADE )",
            function (err, rows, fields) {
            if (err) console.log(err);
        });
    }
    res.send({
        success:true,
        "errMsg":"AHHHHHH"
    });
});
app.get('/dataBaseStatus', function(req , res){
    let fileRows = 0;
    let eventRows = 0;
    let alarmRows = 0;
    connection.query("SELECT COUNT(*) as total FROM ALARM",function(err,rows,fields) {
        if (err){
            throw err;
        }
        for (let row of rows){
            
            alarmRows = row.total;
        }
        connection.query("SELECT COUNT(*) as total FROM EVENT",function(err,rows,fields) {
            if (err){
                throw err;
            }
            for (let row of rows){
               eventRows = row.total;
            }
            connection.query("SELECT COUNT(*) as total FROM FILE",function(err,rows,fields) {
                if (err){
                    throw err;
                
                }
                for (let row of rows){
                    fileRows = row.total;
                }
                res.send({
                    fRows:fileRows,
                    eRows:eventRows,
                    aRows:alarmRows
                });
            });
        });
    });
});
app.get('/closeDatabase', function(req , res){

    res.send("");
});

app.get('/storeFiles', function(req , res){
    let files = fs.readdirSync('./uploads/');
    files.forEach(function(file){
         

        let fileName = "./uploads/"+file;
        let fReturn = calLib.calJSONFromFileName(fileName);

        try {
            JSON.parse(fReturn);
        } catch (e) {
             console.log(fReturn+" error occured while geting "+file+" JSON");
            return;             
        }
        connection.query("SELECT * FROM FILE WHERE(FILE.file_Name='"+file+"')", 
        function (err, rows, fields) {
            if (err){
                console.log(""+err);
            }else {
                console.log(rows.length);     
                if (rows.length == 0){
                
                    let fVal = JSON.parse(fReturn);
                    connection.query("INSERT INTO FILE "
                    +"(file_Name,version,prod_id) "
                    +"VALUES (\""+file+"\",\""+fVal.version+"\",\""+fVal.prodID +"\")",
                    function(err,rows,fields) {
                        if (err){
                            console.log(err);
                        }
                    });
        
                    connection.query("SELECT cal_id FROM FILE ORDER BY cal_id DESC LIMIT 1",
                    function(err,rows,fields) {
                        if (err){
                            throw err;
                        }
                        let calID;
                        for (let row of rows){
                        
                            calID =row.cal_id;
                        }
         
                        console.log("File:"+fileName);
                        let eReturn = calLib.eventListJSONFromFileName(fileName);
                        try {
                            JSON.parse(eReturn);
                        } catch (e) {
                             console.log(eReturn+" error occured while geting events JSON");
                             return;
                        }
                        let eVal = JSON.parse(eReturn);
                        eVal.forEach(function(Event){
                            let tEvent = Event.numAlarms;   
                            connection.query("INSERT INTO EVENT "
                            +"(summary,start_time,location,organizer,cal_file) "
                            +"VALUES (\""+Event.summary+"\",\""
                            +convSqlDT(Event.startDT.date,Event.startDT.time)+"\",\""
                            +Event.loc+"\",\""+ Event.org+"\","+calID +")",
                            function(err,rows,fields) {
                                if (err){
                                    throw err;
                                    return;
                                }

                            });
                            connection.query("SELECT event_id FROM EVENT "+
                            "ORDER BY event_id DESC LIMIT 1",
                            function(err,rows,fields) {
                                if (err){
                                    throw err;
                                }
                                let lastEvent;
                                for (let row of rows){
                                    lastEvent = row.event_id;
                                }
                                for (let i = 1;i<=tEvent; i++){
                                    let aReturn = calLib.alarmListJSONFromFileName(fileName,i+"");       
                                    try {
                                        JSON.parse(aReturn);
                                    } catch (e) {
                                         console.log(aReturn+" error occured while geting alarms JSON");
                                         continue;
                                    }
                                    let aVal = JSON.parse(aReturn);
            
                                    aVal.forEach(function(alarm){
                                        connection.query("INSERT INTO ALARM"
                                        +"(action,`trigger`, event) "
                                        +"VALUES (\""+alarm.action+"\",\""+alarm.trigger+"\","+lastEvent+")",
                                         function(err,rows,fields) {
                                            if (err){
                                                throw err;
                                            }
                                        });       
                                    });
                                }
                            });
                        });
                    });    
                }else{
                    console.log(file+"Already Exists");
                }
            }
        });
    });
    res.send({success:true});
});


app.get('/clearData', function(req , res){
    connection.query("DELETE  FROM ALARM",function(err,rows,fields) {
        if (err){
            res.send({
                success:false,
                "errMsg":err
            });

            throw err;
        }
        
    });
    connection.query("DELETE  FROM EVENT",function(err,rows,fields) {
        if (err){ 
            res.send({
                success:false,
                "errMsg":err
            });
            throw err;
        }
        
    });
    connection.query("DELETE  FROM FILE",function(err,rows,fields) {
        if (err){ 
            res.send({
                success:false,
                "errMsg":err
            });
            throw err;
        }
        
    });

    res.send({success:true});
});


app.get('/displayEventsFromFile', function(req , res){
    console.log("SHOWING VALUES");    
    connection.query("SELECT * FROM EVENT, FILE WHERE" 
    +"(EVENT.cal_file = FILE.cal_id AND FILE.file_Name='"+req.query.fileName+"')"
    +"ORDER BY start_time", function (err, rows, fields) {
        if (err){
            console.log(""+err);
        }else {
            let events = []; 
            for (let row of rows){

                let EventJSON = {
                    id:row.event_id,
                    startTime : "",
                    sum : "",
                    loc: "",
                    org: ""
                }
                EventJSON.startTime = row.start_time;
                EventJSON.sum = row.summary;
                EventJSON.loc= row.location;
                EventJSON.org= row.organizer;
                events.push(EventJSON);
                console.log(EventJSON);
            }
            res.send({
                success:true,
                "events":events
            });
        
        }
    });
});

app.get('/displayEventsWithOrg', function(req , res){
    console.log("SHOWING VALUES");    
    connection.query("SELECT * FROM EVENT WHERE" 
    +"(EVENT.organizer ='"+req.query.org+"')"
    +"ORDER BY start_time", function (err, rows, fields) {
        if (err){
            console.log(""+err);
        }else {
            let events = []; 
            for (let row of rows){

                let EventJSON = {
                    id:row.event_id,
                    startTime : "",
                    sum : "",
                    loc: "",
                    org: ""
                }
                EventJSON.startTime = row.start_time;
                EventJSON.sum = row.summary;
                EventJSON.loc= row.location;
                EventJSON.org= row.organizer;
                events.push(EventJSON);
                console.log(EventJSON);
            }
            res.send({
                success:true,
                "events":events
            });
        
        }
    });
});

app.get('/displayEventsWithLoc', function(req , res){
    console.log("SHOWING VALUES");    
    connection.query("SELECT * FROM EVENT WHERE" 
    +"(EVENT.location ='"+req.query.loc+"')"
    +"ORDER BY start_time", function (err, rows, fields) {
        if (err){
            console.log(""+err);
        }else {
            let events = []; 
            for (let row of rows){

                let EventJSON = {
                    id:row.event_id,
                    startTime : "",
                    sum : "",
                    loc: "",
                    org: ""
                }
                EventJSON.startTime = row.start_time;
                EventJSON.sum = row.summary;
                EventJSON.loc= row.location;
                EventJSON.org= row.organizer;
                events.push(EventJSON);
                console.log(EventJSON);
            }
            res.send({
                success:true,
                "events":events
            });
        
        }
    });
});

app.get('/displayEvents', function(req , res){
   
    connection.query("SELECT * FROM EVENT ORDER BY start_time",
    function (err, rows, fields) {
        if (err){
            console.log(""+err);
        }else {
            let events = []; 
            for (let row of rows){

                let EventJSON = {
                    id:row.event_id,
                    startTime : "",
                    sum : "",
                    loc: "",
                    org: ""
                }
                EventJSON.startTime = row.start_time;
                EventJSON.sum = row.summary;
                EventJSON.loc= row.location;
                EventJSON.org= row.organizer;
                events.push(EventJSON);
                console.log(EventJSON);
            }
            res.send({
                success:true,
                "events":events
            });
        }
    });
});
app.get('/displayConflictEvents', function(req , res){
   
    connection.query("SELECT a.* FROM EVENT a "+
    "JOIN (SELECT start_time, COUNT(*) FROM EVENT "+
    "GROUP BY start_time HAVING COUNT(start_time) > 1) b "+
    "ON a.start_time = b.start_time ORDER by a.start_time",
    function (err, rows, fields) {
        if (err){
            console.log(""+err);
        }else {

            let events = []; 
            for (let row of rows){

                let EventJSON = {
                    id:row.event_id,
                    startTime : "",
                    sum : "",
                    loc: "",
                    org: ""
                }
                EventJSON.startTime = row.start_time;
                EventJSON.sum = row.summary;
                EventJSON.loc= row.location;
                EventJSON.org= row.organizer;
                events.push(EventJSON);
                console.log(EventJSON);
            }
            res.send({
                success:true,
                "events":events
            });


        }
    });
});
app.get('/displayAlarms', function(req , res){
   
    connection.query("SELECT * FROM ALARM ORDER BY event",
    function (err, rows, fields) {
        if (err){
            console.log(""+err);
        }else {
            let alarms = []; 
            for (let row of rows){

                let alarmJSON = {
                    action: "",
                    trigger: ""
                }
                alarmJSON.action= row.action;
                alarmJSON.trigger= row.trigger;
                alarms.push(alarmJSON);
                console.log(alarmJSON);
            }
            res.send({
                success:true,
                "alarms":alarms
            });
        }
    });
});

app.get('/displayAlarmsByEvent', function(req , res){
       
    connection.query("SELECT * FROM ALARM WHERE(ALARM.event ="+req.query.eventId
    +") ORDER BY event",
    function (err, rows, fields) {
        if (err){
            console.log(""+err);
        }else {
            let alarms = []; 
            for (let row of rows){

                let alarmJSON = {
                    action: "",
                    trigger: ""
                }
                alarmJSON.action= row.action;
                alarmJSON.trigger= row.trigger;
                alarms.push(alarmJSON);
                console.log(alarmJSON);
            }
            res.send({
                success:true,
                "alarms":alarms
            });
        }
    });
});

app.get('/getFileNamesInUpload', function(req , res){
   let files = fs.readdirSync('./uploads/');
   console.log(req.query.fileName);
   res.send(JSON.stringify(files));
});

//Sample endpoint
app.get('/getCalJSON', function(req , res){
    console.log(req.query.fileName);
    let fileName = "./uploads/"+req.query.fileName;
    let returnVal = calLib.calJSONFromFileName(fileName);
    //let returnVal = returnVals;
    try {
        JSON.parse(returnVal);
    } catch (e) {
        res.send({
            success:false,
            "errMsg":returnVal+" error occured while geting "+req.query.fileName+" JSON"
        });    
    }
    console.log(fileName+":"+returnVal);    
    res.send({
        success:true,
        "cal":JSON.stringify(JSON.parse(returnVal))
    });
});
app.get('/getEventListJSON', function(req , res){
    let fileName = "./uploads/"+req.query.fileName;
    let returnVal = calLib.eventListJSONFromFileName(fileName);
    //let returnVal = returnVals;
    try {
        JSON.parse(returnVal);
    } catch (e) {
        res.send({
            success:false,
            "errMsg":returnVal+" error occured while geting "+req.query.fileName+" Events JSON"
        });    
    }
    
    console.log(fileName+":"+returnVal);
    res.send({
        success:true,
        "events":JSON.stringify(JSON.parse(returnVal))
    });
  

});

app.get('/getPropListJSON', function(req , res){
    let fileName = "./uploads/"+req.query.fileName;
    let returnVal = calLib.propListJSONFromFileName(fileName,req.query.eventNum);
    //let returnVal = returnVals;
    try {
        JSON.parse(returnVal);
    } catch (e) {
        res.send({
            success:false,
            "errMsg":returnVal+" error occured while geting "+req.query.fileName+" Events JSON"
        });    
    }
    console.log(fileName+":"+returnVal);
    res.send({
        success:true,
        "props":returnVal
    });
 });
app.get('/getAlarmListJSON', function(req , res){
    let fileName = "./uploads/"+req.query.fileName;
    let returnVal = calLib.alarmListJSONFromFileName(fileName,req.query.eventNum);
    //let returnVal = returnVals;
    console.log(fileName+":"+returnVal);

    try {
        JSON.parse(returnVal);
    } catch (e) {
        res.send({
            success:false,
            "errMsg":returnVal+" error occured while geting "+req.query.fileName+" Events JSON"
        });    
    }
    console.log(fileName+":"+returnVal);
    res.send({
        success:true,
        "alarms":JSON.stringify(JSON.parse(returnVal))
    });
});


app.get('/newCal', function(req , res){
   console.log(req.query);
    let valid = validateEventInput(req.query);
    if (req.query.prodId.isNull || req.query.prodId ==""){
        valid = "bad prodId";
    }

    if (req.query.version.isNull || req.query.version ==""){
        valid = "bad version";
    }

    if (req.query.fileName.isNull || req.query.fileName == "" ){
        valid = "bad fileName";
    }else{
        if (/(^.*)?\.(ics)$/.test(req.query.fileName) == false){
            let extend =req.query.fileName.split(".");
            if (extend ==req.query.fileName ){
                req.query.fileName += ".ics";
                console.log(req.query.fileName);
            }else{
                valid = "bad file Extention";
            }
        }
    }
    console.log("User input is "+valid);
    if(valid != "valid" ) {
        return res.send({
            success:false,
            errMsg:'New Calendar Form has '+valid+' no new Cal will be created'
        });
    }
    //event JSON
    if (req.query.startTime.match(/^([0-9]|0[0-9]|1[0-9]|2[0-3]):[0-5][0-9]$/)){
        req.query.startTime = req.query.startTime +":00";
    }
    let EventJSON = {
        UID : req.query.UID,
        startDate : req.query.startDate.replace(/(\/|-)/,"").replace(/(\/|-)/,""),
        startTime : req.query.startTime.replace(/(:|\.)/,"").replace(/(:|\.)/,""),
        UTC : req.query.isUTC,
        creationDate : req.query.curDate,
        creationTime : req.query.curTime,
        sum : req.query.summary
    }
    console.log(JSON.stringify(EventJSON));
    //calJSON
    let CalJSON ={
        version: parseInt(req.query.version),
        prodID : req.query.prodId
    }
    console.log(JSON.stringify(CalJSON));
    let fileName = "./uploads/"+req.query.fileName;
   
    if (fs.existsSync(fileName)){
        fs.unlink(fileName, (err) => {
            if (err) throw err;
            console.log(fileName +' was overwritten');
        });
    }
    console.log(fileName+":");
    let returnVal = calLib.createCalFileFromJSON(fileName,JSON.stringify(CalJSON),JSON.stringify(EventJSON));
    //let returnVal = returnVals;
    console.log(returnVal);
    if(returnVal == "Ok"){    
        res.send({success:true});
    }else{
        res.send({
        success:false,
        "errMsg":returnVal+" error occured while geting "+req.query.fileName+" Events JSON"
        });
    }

});

app.get('/newEventToCal', function(req , res){
    console.log(req.query);
    let valid = validateEventInput(req.query);
    
    if (req.query.fileName == "No Files"){
        valid = "No file selected";
    }
    console.log("User Input is "+valid);

    if(valid!="valid") {
        return res.send({
            success:false,
            errMsg:'Event Form has '+valid+' no event will be added to Cal'
        });
    }
    if (req.query.startTime.match(/^([0-9]|0[0-9]|1[0-9]|2[0-3]):[0-5][0-9]$/)){
        req.query.startTime = req.query.startTime +":00";
    }
    let EventJSON = {
        UID : req.query.UID,
        startDate : req.query.startDate.replace(/(\/|-)/,"").replace(/(\/|-)/,""),
        startTime : req.query.startTime.replace(/(:|\.)/,"").replace(/(:|\.)/,""),
        UTC : req.query.isUTC,
        creationDate : req.query.curDate,
        creationTime : req.query.curTime,
        sum : req.query.summary
    }
    console.log(JSON.stringify(EventJSON));
    let fileName = "./uploads/"+req.query.fileName;
    let returnVal = calLib.addEventToCalFileFromJSON(fileName,JSON.stringify(EventJSON));
    //let returnVal = returnVals;
    console.log(returnVal);
    if(returnVal == "Ok"){    
        res.send({success:true});
    }else{
        res.send({
        success:false,
        "errMsg":returnVal+" error occured while geting "+req.query.fileName+" Events JSON"
        });
    }
});


function validateEventInput(input){
    if (input.UID.isNull || input.UID==""){
        return "bad UID";
    }
    if (/\d{4}(\/|\-)(0?[1-9]|1[012])(\/|\-)(0?[1-9]|[12][0-9]|3[01])$/.test(input.startDate) == false){
        return "bad date format";
    }
    if (/^([0-9]|0[0-9]|1[0-9]|2[0-3]):[0-5][0-9]((:|\.)[0-5][0-9])?$/.test(input.startTime)== false){
        return "bad time format";
    }
    return "valid";
}
function convSqlDT(date,time){
    return date.slice(0, 4) + "-" + date.slice(4,6) +"-"+date.slice(6)+
        "T"+time.slice(0, 2) + ":" + time.slice(2,4) +":"+time.slice(4);
}

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);
