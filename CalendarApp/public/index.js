// Put all onload AJAX calls here, and event listeners
/*$( window ).unload(function() {
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/closeDatabase'
        success: function (file) {
            console.log("Page unloaded");
        },
        fail: function(error) {
                // Non-200 return, do something with error
            console.log(error); 
        }
    });
});
*/
$(document).ready(function() {
    //Loading
    updateStatus("Page Loaded");
    getDataBaseLogin();

    loadAndDisplayFileLog();
    

    // Clearing status panel
    $('#clearStatus').submit(function(e){
        $('#Status').html("");

        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });

    $('#uploadForm').submit(function(e){
         $.ajax({
            url: '/upload',
            type: 'POST',            //Request type       //Data type - we will use JSON for almost everything 
            data: new FormData($('#uploadForm')[0]),
            cache: false,
            contentType: false,
            processData: false,
            xhr: function(){
                return $.ajaxSettings.xhr();
            },
            success: function (file) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
                let fileName = document.getElementById('fileIn').files[0].name;
            //file.files.uploadFile;
                updateStatus(fileName+" Successfuly uploaded to uploads folder");
                console.log(document.getElementById('fileIn').files[0].name+" Loaded");
                loadAndDisplayFileLog();
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });
        
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });
    $('#calViewSelect').submit(function(e){
        let fileName = $("#viewFileSelect option:selected").text();
        //file.files.uploadFile;
        updateStatus("File "+fileName+" Successfully selected");
        console.log(fileName+" Selected");
//        $('#calViewSelect select').append("<option value=\""+fileName+"\">"+fileName+"</option>");
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/getEventListJSON',   //The server endpoint we are connecting to
            data: {
                fileName:fileName
                //$("#viewiFileSelect option:selected").text()
            },
            success: function (data) {
                if(data.success){
                    for(var i = document.getElementById("calViewATable").rows.length; i > 1;i--){
                        document.getElementById("calViewATable").deleteRow(i -1);
                    }
                    clearAlarms();
                    clearProps();
                    updateStatus(fileName+" events successfully retrived");
                        //We write the object to the console to show that the request was successful
                    console.log(data);
                    let eventNo = 1;
                    let utc = "";
                    let list = JSON.parse(data.events);
                    list.forEach(function(item){
                        if(item.startDT.isUTC){
                            utc =" (UTC)";
                        };
                        let markup = "<tr><td>"+eventNo+"</td><td>"
                            + addDateVals(item.startDT.date) +"</td><td>"
                            + addTimeVals(item.startDT.time)+utc+"</td><td>"
                            + item.summary+"</td><td>"
                            + item.numProps+"</td><td>"
                            + item.numAlarms+ "</td><td>"
                            + "<button type=\"button\" onclick=\"showProps("+eventNo+")\">Show Props</button>"+"</td><td>"
                            + "<button type=\"button\" onclick=\"showAlarms("+eventNo+")\">Show Alarms</button>"
                            +"</td></tr>";
                        $("#calViewTable table tbody").append(markup);
                        eventNo++;
                        utc = "";
                    });
                }else{
                    updateStatus(data.errMsg);
                }
            },
            fail: function(error) {
                                       // Non-200 return, do something with error
                console.log(error);
            }
        });

        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });
    $('#newCalVals').submit(function (e){
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/newCal', 
            data:{
                fileName:document.getElementById('newFileNameIn').value,
                version:document.getElementById('newVersionIn').value,
                prodId:document.getElementById('newProdIDIn').value,
                UID: document.getElementById('UIDIn').value,
                startDate: document.getElementById('startDateIn').value,
                startTime: document.getElementById('startTimeIn').value,
                isUTC: document.getElementById('isUID').checked,
                summary: document.getElementById('sumIn').value,
                curDate: getCurrentDate(),
                curTime: getCurrentTime()
            },
            success: function (state) {
                if (state.success){
                    updateStatus("New Calendar called :"
                    + state.fileName,
                    + " Successfully Created"); 
                    loadAndDisplayFileLog();
                //We write the object to the console to show that the request was successful
                }else{
                    updateStatus(state.errMsg);
                }
                            },
            fail: function(error) {
               // Non-200 return, do something with error
                console.log(error); 
            }
        });
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});

    });
    $('#newEventVals').submit(function (e){
        
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/newEventToCal', 
            data:{
                fileName: $("#viewEventFileSelect option:selected").text(),
                UID: document.getElementById('UIDIn').value,
                startDate: document.getElementById('startDateIn').value,
                startTime: document.getElementById('startTimeIn').value,
                isUTC: document.getElementById('isUID').checked,
                summary: document.getElementById('sumIn').value,
                curDate: getCurrentDate(),
                curTime: getCurrentTime()
            },
            success: function (state) {
                if (state.success){
                    updateStatus("New Event Successfully Added to "+$("#viewEventFileSelect option:selected").text()); 
                }else{
                    updateStatus(state.errMsg);
                }

            },
            fail: function(error) {
                // Non-200 return, do something with error
                
                console.log(error); 
            }
        });
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});

    });
/*****************************A4 page querys****************************************/
    $('#displayStatus').submit(function(e){
        getDBStatus();

        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });
    $('#storeFiles').submit(function(e){
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/storeFiles', 
            
            success: function (state) {
                updateStatus("Files stored in database");
                getDBStatus();
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });

        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });
    $('#clearData').submit(function(e){
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/clearData', 
            
            success: function (state) {
                if (state.success){
                    updateStatus("Database data cleared");
                    getDBStatus();
                }else{
                    updateStatus(state.errMsg+"Occured while trying to clear database");
                }
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });

        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });
    $('#displayEvents').submit(function(e){
        $('#displayQuery').html("<h4>All Events in Data Base</h4>"
            +"<table style =\"width:100%\" id = \"qEventViewTable\">" 
            +"<tr><th>Start Time</th>"
            +"<th>Summary</th>"
            +"<th>Location</th>"
            +"<th>Organizer</th>"
            +"<th>Alarm Query</th></tr>"
        );

        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/displayEvents', 
            
            success: function (state) {
                if (state.success){
                    updateStatus("Displaying events from Database");
                    console.log(state.events);
                    state.events.forEach(function(item){
                        if(item.sum == ""){
                            item.sum = "N/A";
                        }

                        if(item.loc == ""){
                            item.loc = "N/A";
                        }
                        if(item.org == ""){
                            item.org = "N/A";
                        }

                        let markup = "<tr><td>"+item.startTime+"</td><td>"
                            +item.sum+"</td><td>"
                            +item.loc+"</td><td>"
                            + item.org+"</td><td>"
                            + "<button type=\"button\" onclick=\"qAlarms("+item.id+")\">Alarms Query</button>"
                            +"</td><tr>";
                        $("#qEventViewTable tbody").append(markup);
                    });

                    getDBStatus();
                }else{
                    updateStatus(state.errMsg+"Occured while trying to clear database");
                }
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });

        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });
    $('#displayConflicts').submit(function(e){
        $('#displayQuery').html("<h4>All Events that Conflict in Data Base</h4>"
            +"<table style =\"width:100%\" id = \"qEventViewTable\">" 
            +"<tr><th>Start Time</th>"
            +"<th>Summary</th>"
            +"<th>Location</th>"
            +"<th>Organizer</th>"
            +"<th>Alarm Query</th></tr>"
        );

        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/displayConflictEvents', 
            
            success: function (state) {
                if (state.success){
                    updateStatus("Displaying events from Database");
                    console.log(state.events);
                    state.events.forEach(function(item){
                        if(item.sum == ""){
                            item.sum = "N/A";
                        }

                        if(item.loc == ""){
                            item.loc = "N/A";
                        }
                        if(item.org == ""){
                            item.org = "N/A";
                        }

                        let markup = "<tr><td>"+item.startTime+"</td><td>"
                            +item.sum+"</td><td>"
                            +item.loc+"</td><td>"
                            + item.org+"</td><td>"
                            + "<button type=\"button\" onclick=\"qAlarms("+item.id+")\">Alarms Query</button>"
                            +"</td><tr>";
                        $("#qEventViewTable tbody").append(markup);
                    });

                    getDBStatus();
                }else{
                    updateStatus(state.errMsg+"Occured while trying to clear database");
                }
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });

        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });

    $('#qEventViewSelect' ).submit(function(e){
        let fileName = $("#qEventViewSelect option:selected").text();
        //file.files.uploadFile;
        updateStatus("File "+fileName+" Successfully selected");
        console.log(fileName+" Selected");

        
        $('#displayQuery').html("<h4>All Events in "+fileName+"</h4>"
            +"<table style =\"width:100%\" id = \"qEventViewTable\">" 
            +"<tr><th>Start Time</th>"
            +"<th>Summary</th>"
            +"<th>Location</th>"
            +"<th>Organizer</th>"
            +"<th>Alarm Query</th></tr>"
        );

        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/displayEventsFromFile', 
            data:{
                fileName: fileName
            },    
            success: function (state) {
                if (state.success){
                    updateStatus("Displaying events from Database");
                    console.log(state.events);
                    state.events.forEach(function(item){
                        if(item.sum == ""){
                            item.sum = "N/A";
                        }

                        if(item.loc == ""){
                            item.loc = "N/A";
                        }
                        if(item.org == ""){
                            item.org = "N/A";
                        }

                        let markup = "<tr><td>"+item.startTime+"</td><td>"
                            +item.sum+"</td><td>"
                            +item.loc+"</td><td>"
                            + item.org+"</td><td>"
                            + "<button type=\"button\" onclick=\"qAlarms("+item.id+")\">Alarms Query</button>"
                            +"</td><tr>";
                        $("#qEventViewTable tbody").append(markup);
                    });

                    getDBStatus();
                }else{
                    updateStatus(state.errMsg+"Occured while trying to clear database");
                }
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });
         e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });

    $('#displayAlarms' ).submit(function(e){ 
        
        $('#displayAlarmProp').html("<h4>All Alarms in file </h4>"
            +"<table style =\"width:100%\" id = \"qAlarmViewTable\">" 
            +"<tr><th>Action</th>"
            +"<th>Trigger</th></tr>"
        );

        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/displayAlarms', 
            
            success: function (state) {
                if (state.success){
                    updateStatus("Displaying Alarms from Database");
                    console.log(state.events);
                    state.alarms.forEach(function(item){
                        if(item.trigger == ""){
                            item.trigger = "N/A";
                        }
                        let markup = "<tr><td>"+item.action+"</td><td>"
                            +item.trigger+"</td></tr>"
                        $("#qAlarmViewTable tbody").append(markup);
                    });

                    getDBStatus();
                }else{
                    updateStatus(state.errMsg+"Occured while trying to clear database");
                }
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });


        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });


    $('#qOrgViewSelect').submit(function(e){
        let org =document.getElementById('organizerIn').value
        $('#displayQuery').html("<h4>All Events with "+org+" Organizer value</h4>"
            +"<table style =\"width:100%\" id = \"qEventViewTable\">" 
            +"<tr><th>Start Time</th>"
            +"<th>Summary</th>"
            +"<th>Location</th>"
            +"<th>Organizer</th>"
            +"<th>Alarm Query</th></tr>"
        );

        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/displayEventsWithOrg', 
            data:{
                org: org//document.getElementById('organizerIn').value
            },    
            success: function (state) {
                if (state.success){
                    updateStatus("Displaying events from Database");
                    console.log(state.events);
                    state.events.forEach(function(item){
                        if(item.sum == ""){
                            item.sum = "N/A";
                        }

                        if(item.loc == ""){
                            item.loc = "N/A";
                        }
                        if(item.org == ""){
                            item.org = "N/A";
                        }

                        let markup = "<tr><td>"+item.startTime+"</td><td>"
                            +item.sum+"</td><td>"
                            +item.loc+"</td><td>"
                            + item.org+"</td><td>"
                            + "<button type=\"button\" onclick=\"qAlarms("+item.id+")\">Alarms Query</button>"
                            +"</td><tr>";
                        $("#qEventViewTable tbody").append(markup);
                    });

                    getDBStatus();
                }else{
                    updateStatus(state.errMsg+"Occured while trying to clear database");
                }
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });
         e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });
    $('#qLocViewSelect').submit(function(e){
        let loc = document.getElementById('locationIn').value;

        $('#displayQuery').html("<h4>All Events with "+loc+" location value</h4>"
            +"<table style =\"width:100%\" id = \"qEventViewTable\">" 
            +"<tr><th>Start Time</th>"
            +"<th>Summary</th>"
            +"<th>Location</th>"
            +"<th>Organizer</th>"
            +"<th>Alarm Query</th></tr>"
        );

        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/displayEventsWithLoc', 
            data:{
                loc: loc
            },    
            success: function (state) {
                if (state.success){
                    updateStatus("Displaying events from Database");
                    console.log(state.events);
                    state.events.forEach(function(item){
                        if(item.sum == ""){
                            item.sum = "N/A";
                        }

                        if(item.loc == ""){
                            item.loc = "N/A";
                        }
                        if(item.org == ""){
                            item.org = "N/A";
                        }

                        let markup = "<tr><td>"+item.startTime+"</td><td>"
                            +item.sum+"</td><td>"
                            +item.loc+"</td><td>"
                            + item.org+"</td><td>"
                            + "<button type=\"button\" onclick=\"qAlarms("+item.id+")\">Alarms Query</button>"
                            +"</td><tr>";
                        $("#qEventViewTable tbody").append(markup);
                    });

                    getDBStatus();
                }else{
                    updateStatus(state.errMsg+"Occured while trying to clear database");
                }
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });
         e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({});
    });
/***************************** End of A4 page querys********************************/
 

});
/*****************************A4 functions****************************************/
 
function qAlarms(eventId){
    $('#displayAlarmProp').html("<h4>Alarms for selected Event</h4>"
        +"<table style =\"width:100%\" id = \"qAlarmViewTable\">" 
        +"<tr><th>Action</th>"
        +"<th>Trigger</th></tr>"
    );
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/displayAlarmsByEvent',   //The server endpoint we are connecting to
        data: {
            eventId:eventId
        },
        success: function (data) {
            if(data.success){
                updateStatus("Event with ID "+eventId+" alarms successfully retrived");
                
                data.alarms.forEach(function(item){
                    let markup = "<tr><td>"+item.action+"</td><td>"
                        + item.trigger+"</td><tr>";
                    $("#qAlarmViewTable tbody").append(markup);
                });
                getDBStatus();
            }else{
                updateStatus(data.errMsg);
            }
        },
        fail: function(error) {
                                       // Non-200 return, do something with error
            console.log(error);
        }
    });
}
function getDataBaseLogin(){
    let dataBaseConnected = false;
    let modal = document.getElementById('loginModal');
    modal.style.display = "block";      
    let login = document.getElementById("loginBtn");
    console.log("AHHH"+login);
    login.onclick =function(e){ 
        e.preventDefault();
        console.log("Trying to connect to Database");
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/databaseValues', 
            data:{
                host     : 'dursley.socs.uoguelph.ca',
                user     : document.getElementById('userNameIn').value,
                password : document.getElementById('passWordIn').value,
                database : document.getElementById('loginDatabaseIn').value
            },
            success: function (state) {
                console.log(state);
                if (state.success){
                    dataBaseConnected = true;
                    modal.style.display = "none";
                    console.log("Conected");
               //We write the object to the console to show that the request was successful
                }else{
                   alert(state.errMsg);
                }
            },
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });
    }
    console.log(dataBaseConnected);
    if (dataBaseConnected == true){
        console.log("Close");
    }
    getDBStatus();
}

function getDBStatus(){
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/dataBaseStatus', 
       
        success: function (state) {
            updateStatus("Database has "+state.fRows+" files, "+state.eRows+" events, and "+state.aRows+" alarms");
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });
}

/*****************************End of A4 Functions****************************************/
 
function clearProps(){
    $('#phantomProps').html("");
}
function showProps(eventNum){
    $('#phantomProps').html("<h4>Optional Props for selected Event</h4>"
        +"<table style =\"width:100%\" id = \"propViewTable\">" 
        +"<tr><th>Prop Name</th>"
        +"<th>Prop Descrption</th></tr>"
    );
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getPropListJSON',   //The server endpoint we are connecting to
        data: {
            fileName:$("#viewFileSelect option:selected").text(),
            eventNum:eventNum
        },
        success: function (data) {
            if(data.success){
                updateStatus("Event "+eventNum+" props successfully retrived");
                //We write the object to the console to show that the request was successful
                console.log(data);
                let list = JSON.parse(data.props);
                list.forEach(function(item){
                    let markup = "<tr><td>"+item.propName+"</td><td>"
                        + item.propDescr+"</td><tr>";
                    $("#propViewTable tbody").append(markup);
                });
            }else{
                updateStatus(data.errMsg);
            }
        },
        fail: function(error) {
                                       // Non-200 return, do something with error
            console.log(error);
        }
    });

    $('#phantomProps').append("<button type = \"button\" onclick = \"clearProps()\">"
        +"Clear Props</button>"
    );
}
function clearAlarms(){
    $('#phantomAlarm').html("");
}
function showAlarms(eventNum){
    $('#phantomAlarm').html("<h4>Alarms for selected Event</h4>"
        +"<table style =\"width:100%\" id = \"alarmViewTable\">" 
        +"<tr><th>Action</th>"
        +"<th>Trigger</th></tr>"
    );
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getAlarmListJSON',   //The server endpoint we are connecting to
        data: {
            fileName:$("#viewFileSelect option:selected").text(),
            eventNum:eventNum
        },
        success: function (data) {
            if(data.success){
                updateStatus("Event "+eventNum+" alarms successfully retrived");
                //We write the object to the console to show that the request was successful
                console.log(data);
                let list = JSON.parse(data.alarms);
                console.log(list);
                list.forEach(function(item){
                    let markup = "<tr><td>"+item.action+"</td><td>"
                        + item.trigger+"</td><tr>";
                    $("#alarmViewTable tbody").append(markup);
                });
            }else{
                updateStatus(data.errMsg);
            }
        },
        fail: function(error) {
                                       // Non-200 return, do something with error
            console.log(error);
        }
    });

    $('#phantomAlarm').append("<button type = \"button\" onclick = \"clearAlarms()\">"
        +"Clear Alarms</button>"
    );
}


function loadAndDisplayFileLog() {
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getFileNamesInUpload',   //The server endpoint we are connecting to
        success: function (data) {
            if (Array.isArray(data) && data.length) {
                for(var i = document.getElementById("theFileLogTable").rows.length; i > 1;i--){
                    document.getElementById("theFileLogTable").deleteRow(i -1);
                }
                $('#calViewSelect select').html("");
                $('#qEventViewSelect select').html("");

                $('#newEvent form select').html("");
            }
            data.forEach(function (item){
                let fileName = item;
                
                $.ajax({
                    type: 'get',            //Request type
                    dataType: 'json',       //Data type - we will use JSON for almost everything 
                    data: {
                        fileName:fileName
                    },
                    url: '/getCalJSON',   //The server endpoint we are connecting to
                    success: function (state) {
                        
                        let fileDownload = "./uploads/"+fileName; 
                        if(state.success){
                            $('#calViewSelect select').append("<option value=\""+fileName+"\">"+fileName+"</option>");
                            $('#qEventViewSelect select').append("<option value=\""+fileName+"\">"+fileName+"</option>");

                            $('#newEvent form select').append("<option value=\""+fileName+"\">"+fileName+"</option>");
                 
                            updateStatus(fileName +" values successfully found");
                            let cal = JSON.parse(state.cal);
                            console.log(cal);

                            let markup = "<tr><td><a href=\"" 
                                + fileDownload
                                + "\">"+fileName+"</a></td><td>" 
                                + cal.version +"</td><td>"
                                + cal.prodID+"</td><td>"
                                + cal.numEvents+"</td><td>"
                                + cal.numProps+ "</td></tr>";   
                            $("#fileLogTable table tbody").append(markup);
                        }else{
/*                            let markup = "<tr><td><a href=\"" 
                                + fileDownload
                                + "\">"+fileName+"</a></td><td>" 
                                + "</td><td>"
                                + "Invalid File</td><td>"
                                + "</td><td>"
                                + "</td></tr>";
                            $("#fileLogTable table tbody").append(markup);*/
                            updateStatus(state.errMsg);
                        }
                    },
                    fail: function(error) {
                        console.log(error); 
                    }

                });
            });
        },
        fail: function(error) {
            console.log(error); 
        }

    });
}

function addTimeVals(time){
    return time.slice(0, 2) + ":" + time.slice(2,4) +":"+time.slice(4);
}
function addDateVals(date){
    return date.slice(0, 4) + "/" + date.slice(4,6) +"/"+date.slice(6);
}

function getCurrentDate(){ 
    let currDate = new Date();
    var day = currDate.getDate(); 
    var month = currDate.getMonth()+1; //Months a 0-indexed 
    var year = currDate.getFullYear(); 
    if(day < 10) { 
        day = '0'+day;
    } 
    if(month < 10) { 
        month = '0'+month;
    } 
    let fullDate = year + month + day ; 
    return fullDate; 
}
function getCurrentTime(){
    var today = new Date();
    var curTime = today.getHours()*10000 + today.getMinutes()*100 + today.getSeconds();
    return curTime;
}

function updateStatus(statusMessage){
    $('#Status').append(statusMessage+"<br>"); 
    var element = document.getElementById("Status");
    element.scrollTop = element.scrollHeight;
}


