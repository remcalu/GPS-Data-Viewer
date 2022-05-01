'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app = express();
const path = require("path");
const fileUpload = require('express-fileupload');
const mysql = require('mysql2/promise');

app.use(fileUpload());
app.use(express.static(path.join(__dirname + '/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`
const portNum = process.argv[2];

// Send HTML at root
app.get('/', function(req, res) {
  res.sendFile(path.join(__dirname + '/public/index.html'));
});

// Send Style
app.get('/style.css', function(req, res) {
  res.sendFile(path.join(__dirname + '/public/style.css'));
});

// Send obfuscated JS
app.get('/index.js', function(req, res) {
  fs.readFile(path.join(__dirname + '/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {
      compact: true,
      controlFlowFlattening: true
    });
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

// Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if (!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.uploadFile;
  if (fs.existsSync('uploads/' + uploadFile.name) == true) {
    return res.status(409).send("The file already exists on the server.")
  }

  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if (err) {
      return res.status(500).send(err);
    }
    if (cLibrary.validUploadedFile(uploadFile.name) == "File is invalid!") {
      fs.unlink('uploads/' + uploadFile.name, function(err) {
        if (err) {
          return res.status(500).send(err);
        }
      });
      return res.status(422).send("The file is invalid.")
    }

    let cResponse = cLibrary.gpxToHTML(uploadFile.name);
    let jsonObj = JSON.parse(cResponse);
    jsonObj["filename"] = uploadFile.name;
    let returnJSON = JSON.stringify(jsonObj);

    res.send(returnJSON);
  });
});

// Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req, res) {
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if (err == null) {
      res.sendFile(path.join(__dirname + '/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: ' + err);
      res.send('');
    }
  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);

/* Getting the c functions ready */
let cLibrary = ffi.Library('./libgpxparser', {
  'validUploadedFile': ['string', ['string']],
  'gpxToHTML': ['string', ['string']],
  'rteToHTML': ['string', ['string']],
  'trkToHTML': ['string', ['string']],
  'ptsToHTML': ['string', ['string', 'int']],
  'otherDataToHTML': ['string', ['string', 'int']],
  'changeRouteOrTrack': ['string', ['string', 'string', 'int']],
  'addGPXToUploads': ['string', ['string', 'string']],
  'addRouteToFile': ['string', ['string', 'string', 'string']],
  'findRoutePathsToJSON': ['string', ['string', 'float', 'float', 'float', 'float', 'float']],
  'findTrackPathsToJSON': ['string', ['string', 'float', 'float', 'float', 'float', 'float']],
});

/**********************
 * CODE FOR FILE PANEL *
 **********************/
/* server side AJAX for uploading a new file to the server, and displaying files in the file log panel */
app.get('/loadgpx', function(req, res) {
  var returnArr = [];
  let files = fs.readdirSync('./uploads');
  for (let i = 0; i < files.length; i++) {
    if (files[i] == ".gitkeep") {
      files.splice(i, 1);
    }
  }
  let invalidFileAdjust = 0;

  /* Looping through all the files in the uploads directory and preparing json with valid files to be returned */
  for (var i = 0; i < files.length; i++) {
    let cResponse = cLibrary.gpxToHTML(files[i]);
    if (cResponse == "File is invalid!") {
      invalidFileAdjust++;
      continue;
    }

    let jsonObj = JSON.parse(cResponse);
    jsonObj["filename"] = files[i];
    returnArr[i - invalidFileAdjust] = JSON.stringify(jsonObj);
  }
  res.send(returnArr);
});

/*********************
 * CODE FOR GPX PANEL *
 *********************/
/* server side GPX track/route table dropdown menu item selection and data display */
app.get('/gpxrtetrk/:filename', function(req, res) {
  let file = req.params.filename;
  var returnArr = [];

  /* Checking if a valid file was select, if yes, return json strings of all the routes and tracks */
  if (file != "No Selection") {
    let cResponse;
    let jsonObj;
    cResponse = cLibrary.rteToHTML(file);
    jsonObj = JSON.parse(cResponse);
    returnArr[0] = JSON.stringify(jsonObj);

    cResponse = cLibrary.trkToHTML(file);
    jsonObj = JSON.parse(cResponse);
    returnArr[1] = JSON.stringify(jsonObj);
  }
  res.send(returnArr);
});

/* server side code for viewing gpx data as an alert */
app.get('/viewgpxdata', function(req, res) {
  let file = req.query.filename;
  let index = req.query.index;
  let cResponse;

  /* Error checking for valid file selection */
  if (file == "No Selection") {
    return res.status(423).send("Enter a name")
  }

  /* Error checking for valid row selection */
  if (isNaN(index) || index == 0) {
    return res.status(422).send("Enter an integer value that is greater than 0.")
  }

  /* Calling c function to get other data */
  cResponse = cLibrary.otherDataToHTML(file, index);
  cResponse = cResponse.replace(/(\r\n|\n|\r)/gm, "");

  /* Error checking for if there is no other data */
  if (cResponse == "No GPXData") {
    return res.status(400).send("No GPXData.")
  } else if (cResponse == "{}") {
    return res.status(401).send("No GPXData at that route.")
  }

  res.send(cResponse);
});

/* server side code for renaming a track or route */
app.get('/renamerteortrk/:filename', function(req, res) {
  let file = req.query.filename;
  let index = req.query.index;
  let newName = req.query.newName;
  let cResponse;

  /* Error checking for valid file selection */
  if (file == "No Selection") {
    return res.status(423).send("Enter a name")
  }

  /* Error checking for valid row selection */
  if (isNaN(index) || index == 0) {
    return res.status(422).send("Enter an integer value that is greater than 0.")
  }

  /* Checking if a name is actually entered */
  if (newName.length == 0) {
    return res.status(424).send("Enter a route or track name")
  }

  /* Calling c function to change route or track */
  cResponse = cLibrary.changeRouteOrTrack(file, newName, index);
  if (cResponse == "Row not Found") {
    return res.status(400).send("Row not Found.")
  }

  /* Refresh database if logged in */
  callRefreshDB(file)

  res.send(cResponse);
});

/**********************
 * CODE FOR CREATE GPX *
 **********************/
/* server side code for creating gpx data as an alert */
app.get('/creategpxfile', function(req, res) {
  let file = req.query.filename;
  let version = parseFloat(req.query.version);
  let creator = req.query.creator;
  let oldPath = file;
  let newPath = "uploads/" + file;
  let cResponse;

  /* Error checking for if input was actually entered for file and creator */
  if (file.length == 0 || creator.length == 0) {
    return res.status(424).send("Enter a name")
  }

  /* Error checking for if the file actually exists */
  if (fs.existsSync(newPath) == true) {
    return res.status(409).send("The file already exists on the server.")
  }

  let obj = new Object();
  obj.version = version;
  obj.creator = creator;
  let jsonString = JSON.stringify(obj);

  /* Error checking for if the file is a .gpx file */
  if (file.endsWith(".gpx")) {
    cResponse = cLibrary.addGPXToUploads(jsonString, "uploads/" + file);

    /* Preparing json for return so that the table can be updated */
    cResponse = cLibrary.gpxToHTML(oldPath);
    let jsonObj = JSON.parse(cResponse);
    jsonObj["filename"] = file;
    jsonString = JSON.stringify(jsonObj);
  } else {
    return res.status(425).send("File extension")
  }

  res.send(jsonString);
});


/*********************
 * CODE FOR ADD ROUTE *
 *********************/
/* server side code adding a waypoint to the client array of waypoints */
app.get('/addwaypoint', function(req, res) {
  let latitude = parseFloat(req.query.latitude);
  let longitude = parseFloat(req.query.longitude);

  /* Error checking correct latitude */
  if (latitude < -90.0 || latitude > 90.0 || isNaN(latitude)) {
    return res.status(450).send("Invalid latitude.")
  }

  /* Error checking correct longitude */
  if (longitude < -180.0 || longitude >= 180.0 || isNaN(longitude)) {
    return res.status(451).send("Invalid longitude.")
  }

  let obj = new Object();
  obj.latitude = latitude;
  obj.longitude = longitude;
  let jsonString = JSON.stringify(obj);

  res.send(jsonString);
});

/* server side code adding a route with its waypoints */
app.get('/addroute', function(req, res) {
  let file = req.query.filename;
  let routeName = req.query.routeName;
  let obj = new Object();
  obj.name = routeName;
  routeName = JSON.stringify(obj);
  let waypointArray = req.query.waypointArray;
  let jsonString = JSON.stringify(waypointArray);
  let oldPath = file;
  let newPath = "uploads/" + file;
  let cResponse;

  /* Error checking for if a file is chosen or not */
  if (file == "No Selection") {
    return res.status(410).send("Select a file.")
  }

  /* Error checking for if the file actually exists */
  if (fs.existsSync(newPath) == false) {
    return res.status(409).send("The file doesnt exist.")
  }

  /* Error checking for if the file is a .gpx file */
  if (file.endsWith(".gpx")) {
    cLibrary.addRouteToFile(file, jsonString, routeName);

    /* Setting new directory of the file */
    fs.rename(oldPath, newPath, function(err) {
      if (err) throw err
    })

    /* Preparing json for return so that the table can be updated */
    cResponse = cLibrary.gpxToHTML(oldPath);
    let jsonObj = JSON.parse(cResponse);
    jsonObj["filename"] = file;
    jsonString = JSON.stringify(jsonObj);
  } else {
    return res.status(425).send("File extension")
  }

  /* Refresh database if logged in */
  callRefreshDB(file);

  res.send(jsonString);
});

/**********************
 * CODE FOR FIND PATHS *
 **********************/
/* server side code adding a waypoint to the client array of waypoints */
app.get('/findpaths', function(req, res) {
  let latStart = parseFloat(req.query.latStart);
  let lonStart = parseFloat(req.query.lonStart);
  let latEnd = parseFloat(req.query.latEnd);
  let lonEnd = parseFloat(req.query.lonEnd);
  let delta = parseFloat(req.query.delta);
  let cResponseArr = [];

  /* Error checking valid start latitude */
  if (latStart < -90.0 || latStart > 90.0 || isNaN(latStart)) {
    return res.status(450).send("Invalid latitude.")
  }

  /* Error checking valid start longitude */
  if (lonStart < -180.0 || lonStart >= 180.0 || isNaN(lonStart)) {
    return res.status(451).send("Invalid longitude.")
  }

  /* Error checking valid end latitude */
  if (latEnd < -90.0 || latEnd > 90.0 || isNaN(latEnd)) {
    return res.status(450).send("Invalid latitude.")
  }

  /* Error checking valid end longitude */
  if (lonEnd < -180.0 || lonEnd >= 180.0 || isNaN(lonEnd)) {
    return res.status(451).send("Invalid longitude.")
  }

  /* Error checking the delta */
  if (delta < 0 || isNaN(delta)) {
    return res.status(452).send("Invalid delta.")
  }

  let files = fs.readdirSync('./uploads');
  for (let i = 0; i < files.length; i++) {
    if (files[i] == ".gitkeep") {
      files.splice(i, 1);
    }
  }
  files = JSON.stringify(files);
  cResponseArr[0] = cLibrary.findRoutePathsToJSON(files, latStart, lonStart, latEnd, lonEnd, delta);
  cResponseArr[1] = cLibrary.findTrackPathsToJSON(files, latStart, lonStart, latEnd, lonEnd, delta);
  res.send(cResponseArr);
});

/* Database */
let credentials = {};
let loggedin = false;
let connection; 
let queryFile = `CREATE TABLE IF NOT EXISTS FILE (
  gpx_id INT AUTO_INCREMENT PRIMARY KEY, 
  file_name VARCHAR(60) NOT NULL, 
  ver DECIMAL(2,1) NOT NULL, 
  creator VARCHAR(256) NOT NULL)`;
    
let queryRoute = `CREATE TABLE IF NOT EXISTS ROUTE (
	route_id INT AUTO_INCREMENT PRIMARY KEY, 
	route_name VARCHAR(256), 
	route_len FLOAT(15,7) NOT NULL, 
	gpx_id INT NOT NULL,
	FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE)`;
	
let queryPoint = `CREATE TABLE IF NOT EXISTS POINT (
	point_id INT AUTO_INCREMENT PRIMARY KEY, 
	point_index INT NOT NULL, 
	latitude DECIMAL(11,7) NOT NULL, 
	longitude DECIMAL(11,7) NOT NULL, 
	point_name VARCHAR(256), 
	route_id INT NOT NULL,
	FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE)`;

/* server side code for logging in */
app.get('/dblogin', async function(req, res) {
  let numFilesInTable;
  let files = fs.readdirSync('./uploads');
  for (let i = 0; i < files.length; i++) {
    if (files[i] == ".gitkeep") {
      files.splice(i, 1);
    }
  }

  /* Setup the credentials */
	credentials = {
    host: "dursley.socs.uoguelph.ca", 
    user: req.query.username, 
    password: req.query.password, 
    database: req.query.dbname
  }

  /* Create connection and execute various queries, and catch any errors */
	try {  	
		connection = await mysql.createConnection(credentials);
		await connection.execute(queryFile);
		await connection.execute(queryRoute);
		await connection.execute(queryPoint);

    /* Counting number of files in table */
    let [frow, fields1] = await connection.execute('SELECT (SELECT COUNT(*) FROM FILE) AS fc;');
    for (let row of frow) {
      numFilesInTable = row.fc;
    }
	} catch(error) {  
    console.log("Error: '" + error + "' caught");
    return res.status(450).send("Failure")
	}

  if (connection && connection.end) {
    connection.end();  
  }

  /* Counting number of files in table and files on server */
  let returnStr;
  let jsonObj = new Object();
  jsonObj["tablefiles"] = numFilesInTable;
  jsonObj["regfiles"] = files.length;
  returnStr = JSON.stringify(jsonObj);
  loggedin = true;
  res.send(returnStr);
});

/* server side code for refreshing the data in the database */
app.get('/dbrefresh', async function(req, res) {
	let rowvals = [];

  /* Create connection and count the amount of data, and catch any errors */
	try {  	
		connection = await mysql.createConnection(credentials);
		let [frow, fields1] = await connection.execute('SELECT (SELECT COUNT(*) FROM FILE) AS fc;');
		let [rrow, fields2] = await connection.execute('SELECT (SELECT COUNT(*) FROM ROUTE) AS rc;');
		let [prow, fields3] = await connection.execute('SELECT (SELECT COUNT(*) FROM POINT) AS pc;');
		
		for (let row of frow) {
      rowvals[0] = row.fc;
    }
		for (let row of rrow) {
      rowvals[1] = row.rc;
    }
		for (let row of prow) {
      rowvals[2] = row.pc;
    }
	} catch(error) {  
    console.log("Error: '" + error + "' caught");
    return res.status(450).send("Failure")
			
	}

  if (connection && connection.end) {
    connection.end();  
  }
	
	res.send(rowvals);
});

/* server side code for storing data */
app.get('/dbstore', async function(req, res) {
  let numFilesInTable = 0;
  let files = fs.readdirSync('./uploads');
  for (let i = 0; i < files.length; i++) {
    if (files[i] == ".gitkeep") {
      files.splice(i, 1);
    }
  }

  try {  	
		connection = await mysql.createConnection(credentials);
		for (let i = 0; i < files.length; i++) {
			let cResponse = cLibrary.gpxToHTML(files[i]);
      let [rows1, fields1] = await connection.execute("SELECT COUNT(1) FROM FILE WHERE file_name='" + files[i] + "'");
			if (cResponse == "File is invalid!" || rows1[0]["COUNT(1)"] != 0) {
        continue;
      }

      /* Inserting data regarding the current document being entered */
			let gpxObj = JSON.parse(cResponse);
			gpxObj["filename"] = files[i];
			let sqlData = 'INSERT IGNORE INTO FILE (file_name, ver, creator) VALUES (\'' + gpxObj["filename"] + '\',\'' + gpxObj["version"] + '\',\'' + gpxObj["creator"] +'\');';
			await connection.execute(sqlData);
			
      /* Inserting data regarding routes into the database */
			cResponse = cLibrary.rteToHTML(files[i]);
			let rteObj = JSON.parse(cResponse);
			for (let j = 0; j < rteObj.length; j++) {
				sqlData = 'SELECT (SELECT gpx_id FROM FILE WHERE file_name=\''+files[i]+'\') AS gpx;';
				let [rows, fields] = await connection.execute(sqlData);
        if (rteObj[j]["name"] == "" || rteObj[j]["name"] == "None") {
          rteObj[j]["name"] = "NULL";
        }
				sqlData = 'INSERT IGNORE INTO ROUTE (route_name, route_len, gpx_id) VALUES (\'' + rteObj[j]["name"] + '\',\'' + rteObj[j]["len"] + '\', \'' + rows[0].gpx + '\');';
				await connection.execute(sqlData);

        /* Inserting data regarding points into the database */
        sqlData = 'SELECT LAST_INSERT_ID();';
				let [rows2, fields2] = await connection.execute(sqlData);
        cResponse = cLibrary.ptsToHTML(files[i], j);
        let ptObj = JSON.parse(cResponse);
				for (let k = 0; k < ptObj.length; k++) {
          if (ptObj[k]["point_name"] == "") {
            ptObj[k]["point_name"] = "NULL";
          }
					sqlData = 'INSERT IGNORE INTO POINT (point_index, longitude, latitude, point_name, route_id) VALUES (\'' + ptObj[k]["point_index"] + '\',\'' + ptObj[k]["longitude"] + '\', \'' + ptObj[k]["latitude"] + '\',\'' + ptObj[k]["point_name"] + '\',\'' + rows2[0]['LAST_INSERT_ID()'] + '\');';
					await connection.execute(sqlData);
				}
			}
		}

    /* Counting number of files in table */
    let [frow, fields1] = await connection.execute('SELECT (SELECT COUNT(*) FROM FILE) AS fc;');
    for (let row of frow) {
      numFilesInTable = row.fc;
    }
	} catch(error) {  
    console.log("Error: '" + error + "' caught");
    return res.status(450).send("Failure")
	}

  /* Counting number of files in table */
  let returnStr;
  let jsonObj = new Object();
  jsonObj["tablefiles"] = numFilesInTable;
  returnStr = JSON.stringify(jsonObj);

  if (connection && connection.end) {
    connection.end();  
  }

  res.send(returnStr);
});

/* server side code for clearing data */
app.get('/dbcleardata', async function(req, res) {

  /* Create connection and clear all data, and catch any errors */
	try {  	
		connection = await mysql.createConnection(credentials);
		await connection.execute("DELETE FROM POINT");
		await connection.execute("DELETE FROM ROUTE");
		await connection.execute("DELETE FROM FILE");
	} catch(error) {  
    console.log("Error: '" + error + "' caught");
    return res.status(450).send("Failure")
	}

	if (connection && connection.end) {
    connection.end();  
	}
	res.send("Success");
});

/* server side code for doing a query on routes */
app.get('/dbqueryroute', async function(req, res) {
  let rowsdata = [];
  let counter = 0;
  let sort;
  if (req.query.filename == "All Files") {
    sort = 'SELECT * from `ROUTE` ORDER BY ' + '`'+ req.query.which +'`;';
  } else {
    sort = 'SELECT * from `ROUTE` WHERE gpx_id=(SELECT gpx_id FROM FILE WHERE file_name=\'' + req.query.filename +'\''+') ORDER BY ' + '`'+ req.query.which +'`;';
  }

  /* Create connection and clear all data, and catch any errors */
	try {  	
		connection = await mysql.createConnection(credentials);
		let [rows, fields] = await connection.execute(sort);
		for (let row of rows) {
      rowsdata[counter] = {"route_name": row.route_name, "route_len": row.route_len};
      counter++;
    }
	} catch(error) {  
    console.log("Error: '" + error + "' caught");
    return res.status(450).send("Failure")
	}

	if (connection && connection.end) {
    connection.end();  
	}

	res.send(rowsdata);
});

/* server side code for doing a query on points */
app.get('/dbquerypoint', async function(req, res) {
  let rowsdata = [];
  let counter = 0;
  let getNames;
  let sort;
  if (req.query.filename == "No Selection") {
    return res.status(451).send("Choose")
  }

  if (req.query.rtename == "") {
    getNames = 'SELECT * from `ROUTE` WHERE gpx_id=(SELECT gpx_id FROM FILE WHERE file_name=\'' + req.query.filename +'\''+') ORDER BY ' + '`'+ req.query.which +'`;';
  } else {
    sort = 'SELECT * from `POINT` WHERE route_id=(SELECT route_id FROM `ROUTE` WHERE gpx_id=(SELECT gpx_id FROM FILE WHERE file_name=\'' + req.query.filename +'\')'+' AND route_name=\''+ req.query.rtename  + '\') ORDER BY `point_index`;';
  }

  /* Create connection and clear all data, and catch any errors */
	try {  	
    connection = await mysql.createConnection(credentials);

    /* QUERY 4 AND 3 */
    if (req.query.rtename == "") {
      let [rows, fields] = await connection.execute(getNames);
      let unknownName = 0;
      /* Loop through every route in the specified file */
      for (let row of rows) {
        sort = 'SELECT * from `POINT` WHERE route_id=(' + row.route_id + ') ORDER BY `point_index`;';
        let [rows2, fields2] = await connection.execute(sort);
        if (row.route_name == "NULL") {
          unknownName++;
        }
        /* Loop through every waypoint in the specified file, according to which route the first loop is current on */
        for (let row2 of rows2) {
          if (row.route_name != "NULL") {
            rowsdata[counter++] = {"route_name": row.route_name, "point_index": row2.point_index, "point_name": row2.point_name, "latitude": row2.latitude, "longitude": row2.longitude};
          } else {
            let generic = "Unnamed route " + unknownName;
            rowsdata[counter++] = {"route_name": generic, "point_index": row2.point_index, "point_name": row2.point_name, "latitude": row2.latitude, "longitude": row2.longitude};
          }
        }
      } 
    } else {
      let [rows, fields] = await connection.execute(sort);
      for (let row of rows) {
        rowsdata[counter++] = {"route_name": req.query.rtename, "point_index": row.point_index, "point_name": row.point_name, "latitude": row.latitude, "longitude": row.longitude};
      } 
    }
	} catch(error) {  
    console.log("Error: '" + error + "' caught");
    return res.status(450).send("Failure")
	}

	if (connection && connection.end) {
    connection.end();  
	}

	res.send(rowsdata);
});

/* server side code for doing a query on shortest/longest routes d*/
app.get('/dbquerylength', async function(req, res) {
  let counter = 0;
  let rowsdata = [];
  let sort;
  req.query.length = parseFloat(req.query.length)
  if (req.query.length < 1 || Number.isInteger(req.query.length) == false) {
    return res.status(451).send("Failure")
  }
  if (req.query.filename == "No Selection") {
    return res.status(452).send("Failure")
  }

  sort = 'SELECT * from `ROUTE` WHERE gpx_id=(SELECT gpx_id FROM FILE WHERE file_name=\'' + req.query.filename +'\''+') ORDER BY route_len;';

  /* Create connection and clear all data, and catch any errors */
	try {  	
		connection = await mysql.createConnection(credentials);
		let [rows, fields] = await connection.execute(sort);
    let routeArr = [];
    if (req.query.shortestLongest == "short") {
      if (req.query.length > rows.length) {
        req.query.length = rows.length;
      }
      for (let i = 0; i < req.query.length; i++) {
        routeArr.push(rows[i]);
      }
    } else if (req.query.shortestLongest == "long") {
      if (req.query.length > rows.length) {
        req.query.length = rows.length;
      }
      for (let i = rows.length-1; i >= rows.length-req.query.length; i--) {
        routeArr.push(rows[i]);
      }
    }

    /* Sort by route name if user chooses so */
    if (req.query.which == "route_name") {
      routeArr.sort((a,b)=> (a.route_name.toLowerCase() > b.route_name.toLowerCase() ? 1 : -1));
    }
    for (let i = 0; i < routeArr.length; i++) {
      rowsdata[i] = {"file_name": req.query.filename, "route_name": routeArr[i].route_name, "route_len": routeArr[i].route_len};
    }
	} catch(error) {  
    console.log("Error: '" + error + "' caught");
    return res.status(450).send("Failure")
	}

	if (connection && connection.end) {
    connection.end();  
	}

	res.send(rowsdata);
});

/* Functions to refresh the database */
async function callRefreshDB(file) {
  if (loggedin == true) {
    try {
      removeRefreshDB(file);
    } catch(error) {
      console.log("Error Caught!\n");
    }
  }
}

/* Functions to refresh the database */
async function removeRefreshDB(file) {
  connection = await mysql.createConnection(credentials);
  let [rowa, fielda] = await connection.execute('SELECT COUNT(1) FROM FILE WHERE file_name = \'' + file + '\'');
  if (rowa[0]['COUNT(1)'] != 0) {
    await connection.execute('DELETE FROM FILE WHERE file_name = \'' + file + '\'');
  } else {
    return;
  }

  let cResponse = cLibrary.gpxToHTML(file);
  let [rows1, fields1] = await connection.execute("SELECT COUNT(1) FROM FILE WHERE file_name='" + file + "'");
  if (cResponse == "File is invalid!" || rows1[0]["COUNT(1)"] != 0) {
    return;
  }

  /* Inserting data regarding the current document being entered */
  let gpxObj = JSON.parse(cResponse);
  gpxObj["filename"] = file;
  let sqlData = 'INSERT IGNORE INTO FILE (file_name, ver, creator) VALUES (\'' + gpxObj["filename"] + '\',\'' + gpxObj["version"] + '\',\'' + gpxObj["creator"] +'\');';
  await connection.execute(sqlData);
  
  /* Inserting data regarding routes into the database */
  cResponse = cLibrary.rteToHTML(file);
  let rteObj = JSON.parse(cResponse);
  for (let j = 0; j < rteObj.length; j++) {
    sqlData = 'SELECT (SELECT gpx_id FROM FILE WHERE file_name=\''+file+'\') AS gpx;';
    let [rows, fields] = await connection.execute(sqlData);
    if (rteObj[j]["name"] == "" || rteObj[j]["name"] == "None") {
      rteObj[j]["name"] = "NULL";
    }
    sqlData = 'INSERT IGNORE INTO ROUTE (route_name, route_len, gpx_id) VALUES (\'' + rteObj[j]["name"] + '\',\'' + rteObj[j]["len"] + '\', \'' + rows[0].gpx + '\');';
    await connection.execute(sqlData);

    /* Inserting data regarding points into the database */
    sqlData = 'SELECT LAST_INSERT_ID();';
    let [rows2, fields2] = await connection.execute(sqlData);
    cResponse = cLibrary.ptsToHTML(file, j);
    let ptObj = JSON.parse(cResponse);
    for (let k = 0; k < ptObj.length; k++) {
      if (ptObj[k]["point_name"] == "") {
        ptObj[k]["point_name"] = "NULL";
      }
      sqlData = 'INSERT IGNORE INTO POINT (point_index, longitude, latitude, point_name, route_id) VALUES (\'' + ptObj[k]["point_index"] + '\',\'' + ptObj[k]["longitude"] + '\', \'' + ptObj[k]["latitude"] + '\',\'' + ptObj[k]["point_name"] + '\',\'' + rows2[0]['LAST_INSERT_ID()'] + '\');';
      await connection.execute(sqlData);
    }
  }

  if (connection && connection.end) {
    connection.end();  
	}
}