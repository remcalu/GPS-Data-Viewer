// Put all onload AJAX calls here, and event listeners
jQuery(document).ready(function() {

    let waypointArray = [];
    let loggedin = false;

    /**********************
     * CODE FOR FILE PANEL *
     **********************/
    /* client side AJAX for for updating places with file names */
    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/loadgpx',

        success: function(data) {
            /* If there are files on the server, display them accordingly */
            for (var i = 0; i < data.length; i++) {
                let json = JSON.parse(data[i]);
                $('#no-files-filler').remove();
                $('#file-log-section-table').append("<tr><th scope=\"row\"><a href=\"uploads/" + json["filename"] + "\">" + json["filename"] + "</a></th><td>" + json["version"] + "</td><td>" + json["creator"] + "</td><td>" + json["numWaypoints"] + "</td><td>" + json["numRoutes"] + "</td><td>" + json["numTracks"] + "</td>");
                $('#gpx-view-panel-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                $('#add-route-panel-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                $('#point-query-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                $('#route-query-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                $('#shortest-longest-query-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                alert('Added ' + json["filename"] + ' to File Log Panel');
            }
        },
        error: function(error) {
            alert('Failed to add ' + json["filename"] + ' to File Log Panel');
        }
    });

    /* client side AJAX for uploading a new file to the server, and displaying files in the file log panel */
    $('#submit-upload-button').submit(function(e) {
        e.preventDefault();
        let form = new FormData(this);

        $.ajax({
            type: 'post',
            dataType: 'json',
            url: '/upload',
            data: form,
            contentType: false,
            cache: false,
            processData: false,

            success: function(data) {
                /* After an upload, update the table */
                let json = data;
                $('#no-files-filler').remove();
                $('#file-log-section-table').append("<tr><th scope=\"row\"><a href=\"uploads/" + json["filename"] + "\">" + json["filename"] + "</a></th><td>" + json["version"] + "</td><td>" + json["creator"] + "</td><td>" + json["numWaypoints"] + "</td><td>" + json["numRoutes"] + "</td><td>" + json["numTracks"] + "</td>");
                $('#gpx-view-panel-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                $('#add-route-panel-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                $('#point-query-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                $('#route-query-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                $('#shortest-longest-query-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                alert('Added ' + json["filename"] + ' to File Log Panel');
                if (loggedin == true) {
                    $('#store').prop('disabled', false);
                }
            },
            error: function(error) {
                if (error.status == 409) {
                    alert("Error, the file already exists");
                } else if (error.status == 422) {
                    alert("Error, the file is invalid");
                } else if (error.status == 400) {
                    alert("Error, select a file to upload");
                } else {
                    alert("Error");
                }
            }
        });
    });

    /*********************
     * CODE FOR GPX PANEL *
     *********************/
    /* client side GPX track/route table dropdown menu item selection and data display */
    $('#gpx-view-panel-dropdown-menu').change(function() {
        let filename = $("#gpx-view-panel-dropdown-menu option:selected").text();
        $('#gpx-view-panel-rte-trk-table tbody tr').remove();

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/gpxrtetrk/' + filename,

            success: function(data) {
                if (data.length != 0) {
                    let jsonArr;
                    jsonArr = JSON.parse(data[0]);
                    for (var i = 0; i < jsonArr.length; i++) {
                        let json = jsonArr[i];
                        $("#gpx-view-panel-rte-trk-table").append("<tr><td> Route " + (i + 1) + "</td><td id=\"rte-" + json["numPoints"] + "\">" + json["name"] + "</td><td>" + json["numPoints"] + "</td><td>" + json["len"] + "m</td><td>" + json["loop"] + "</td></tr>");
                    }

                    jsonArr = JSON.parse(data[1]);
                    for (var i = 0; i < jsonArr.length; i++) {
                        let json = jsonArr[i];
                        $("#gpx-view-panel-rte-trk-table").append("<tr><td> Track " + (i + 1) + "</td><td id=\"rte-" + json["numPoints"] + "\">" + json["name"] + "</td><td>" + json["numPoints"] + "</td><td>" + json["len"] + "m</td><td>" + json["loop"] + "</td></tr>");
                    }
                    alert("Displayed Routes and Tracks from " + filename + " to GPX Panel");
                }
            },
            error: function(error) {
                alert("Could not display Routes or Tracks in " + filename + " to GPX Panel");
            }
        });
    });

    /* client side code for viewing gpx data as an alert */
    $('#gpx-view-confirm').on('click', function() {
        let filename = $("#gpx-view-panel-dropdown-menu option:selected").text();
        let index = parseInt($("#gpx-view-row").val());
        $("#gpx-view-row").val('');

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/viewgpxdata',
            data: {
                filename: filename,
                index: index,
            },

            success: function(data) {
                /* Displaying otherdata summary */
                alert("Other GPXData Summary for row " + index + ":\n\n" + data["otherData"]);
            },
            error: function(error) {
                if (error.status == 400) {
                    alert("Error, Row " + index + " doesnt exist\n");
                } else if (error.status == 401) {
                    alert("No GPXData for row " + index);
                } else if (error.status == 422) {
                    alert("Error, Enter an integer value that is greater than 0.");
                } else if (error.status == 423) {
                    alert("Error, Choose a file");
                } else {
                    alert("Error");
                }
            }
        });
    });

    /* client side code for renaming a track or route */
    $('#gpx-rename-confirm').on('click', function() {
        let filename = $("#gpx-view-panel-dropdown-menu option:selected").text();
        let index = parseInt($("#gpx-rename-row").val());
        let newName = $("#gpx-rename-name").val();
        $("#gpx-rename-row").val('')
        $("#gpx-rename-name").val('')

        $.ajax({
            type: 'get',
            url: '/renamerteortrk/' + filename,
            data: {
                filename: filename,
                index: index,
                newName: newName,
            },

            success: function(data) {
                /* Changing the row names */
                let row = $('#gpx-view-panel-rte-trk-table').find('td');
                index = ((index - 1) * 5) + 1
                for (i = 0; i < row.length * 5; i++) {
                    if (i == index) {
                        row.eq(i).html(newName);
                    }
                }
                index = ((index - 1) / 5) + 1
                alert("Successfully renamed item in row " + index + " to " + newName + "\n");
            },
            error: function(error) {
                if (error.status == 400) {
                    alert("Error, Row " + index + " doesnt exist\n");
                } else if (error.status == 422) {
                    alert("Error, Enter an integer value that is greater than 0.");
                } else if (error.status == 423) {
                    alert("Error, Choose a file");
                } else if (error.status == 424) {
                    alert("Error, Enter a name for the renaming.");
                } else {
                    alert("Error");
                }
            }
        });
    });

    /**********************
     * CODE FOR CREATE GPX *
     **********************/
    /* client side code for creating a new gpx file */
    $('#create-doc-button').on('click', function() {
        let filename = $("#create-filename").val();
        let version = parseFloat($("#create-version option:selected").val());
        let creator = $("#create-creator").val();
        $("#create-filename").val('')
        $("#create-creator").val('')

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/creategpxfile',
            data: {
                filename: filename,
                version: version,
                creator: creator,
            },

            success: function(data) {
                /* Updating the table and drop down menus */
                json = data;
                $('#no-files-filler').remove();
                $('#file-log-section-table').append("<tr><th scope=\"row\"><a href=\"uploads/" + json["filename"] + "\">" + json["filename"] + "</a></th><td>" + json["version"] + "</td><td>" + json["creator"] + "</td><td>" + json["numWaypoints"] + "</td><td>" + json["numRoutes"] + "</td><td>" + json["numTracks"] + "</td>");
                $('#gpx-view-panel-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                $('#add-route-panel-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                $('#point-query-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                $('#route-query-dropdown-menu').append("<option value=\"" + json["filename"] + "\"" + ">" + json["filename"] + "</option>");
                if (loggedin == true) {
                    $('#store').prop('disabled', false);
                }
                alert("Successfully created file " + filename + "\n");
            },
            error: function(error) {
                if (error.status == 409) {
                    alert("Error, the file " + filename + " already exists");
                } else if (error.status == 424) {
                    alert("Error, Fields can not be empty\n");
                } else if (error.status == 425) {
                    alert("Error, File name should end with .gpx\n");
                } else {
                    alert("Error");
                }
            }
        });
    });

    /*********************
     * CODE FOR ADD ROUTE *
     *********************/
    /* client side code for add a new waypoint to a waypoint array */
    $('#add-latitude-longitude-confirm').on('click', function() {
        let latitude = parseFloat($("#add-latitude").val());
        let longitude = parseFloat($("#add-longitude").val());
        $("#add-latitude").val('')
        $("#add-longitude").val('')

        $.ajax({
            type: 'get',
            url: '/addwaypoint',
            dataType: 'json',
            data: {
                latitude: latitude,
                longitude: longitude,
            },

            success: function(data) {
                /* Updating the array of waypoints */
                json = data;
                jsonString = JSON.stringify(data);
                waypointArray.push(jsonString);
                alert("Added waypoint with latitude " + json["latitude"] + " and longitude " + json["longitude"] + " to the next route that'll be added\n");
            },
            error: function(error) {
                if (error.status == 450) {
                    alert("Error, latitude must be such that '-90.0 <= value <= 90.0'");
                } else if (error.status == 451) {
                    alert("Error, longitude must be such that '-180.0 <= value < 180.0'");
                } else {
                    alert("Error");
                }
            }
        });
    });

    /* client side code for add a new route to a gpx file */
    $('#add-route-confirm').on('click', function() {
        let filename = $("#add-route-panel-dropdown-menu option:selected").val();
        let routeName = $("#add-route-name").val();
        $("#add-route-name").val('')

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/addroute',
            data: {
                filename: filename,
                routeName: routeName,
                waypointArray: waypointArray,
            },

            success: function(data) {
                /* Updating the table and drop down menus */

                /* Finding correct row */
                let index;
                let row = $('#file-log-section-table').find('a');
                for (i = 0; i < row.length; i++) {
                    if (row.eq(i).html() == filename) {
                        index = i;
                    }
                }

                /* Finding column row */
                column = $('#file-log-section-table').find('td');
                index = (index + 3) * (index + 1);
                for (i = 0; i < column.length; i++) {
                    if (i == index) {
                        column.eq(i).html(parseInt(column.eq(i).html()) + 1);
                    }
                }
                alert("Successfully added route '" + routeName + "' with " + waypointArray.length + " waypoints!");
                waypointArray = [];
            },
            error: function(error) {
                if (error.status == 409) {
                    alert("Error, the file does not exist");
                } else if (error.status == 410) {
                    alert("Error, choose a file from the drop down list");
                } else {
                    alert("Error");
                }
            }
        });
    });

    /**********************
     * CODE FOR FIND PATHS *
     **********************/
    /* client side code for add a new route to a gpx file */
    $('#find-path-confirm').on('click', function() {
        let latStart = parseFloat($("#lat-start").val());
        let lonStart = parseFloat($("#lon-start").val());
        let latEnd = parseFloat($("#lat-end").val());
        let lonEnd = parseFloat($("#lon-end").val());
        let delta = parseFloat($("#delta-val").val());
        $("#lat-start").val('')
        $("#lon-start").val('')
        $("#lat-end").val('')
        $("#lon-end").val('')
        $("#delta-val").val('')

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/findpaths',
            data: {
                latStart: latStart,
                lonStart: lonStart,
                latEnd: latEnd,
                lonEnd: lonEnd,
                delta: delta,
            },

            success: function(data) {
                if (data.length != 0) {
                    $('#get-path-table tbody tr').remove();
                    let jsonArr;
                    jsonArr = JSON.parse(data[0]);
                    for (var i = 0; i < jsonArr.length; i++) {
                        let json = jsonArr[i];
                        $("#get-path-table").append("<tr><td> Route " + (i + 1) + "</td><td id=\"rte-" + json["numPoints"] + "\">" + json["name"] + "</td><td>" + json["numPoints"] + "</td><td>" + json["len"] + "m</td><td>" + json["loop"] + "</td></tr>");
                    }

                    jsonArr = JSON.parse(data[1]);
                    for (var i = 0; i < jsonArr.length; i++) {
                        let json = jsonArr[i];
                        $("#get-path-table").append("<tr><td> Track " + (i + 1) + "</td><td id=\"rte-" + json["numPoints"] + "\">" + json["name"] + "</td><td>" + json["numPoints"] + "</td><td>" + json["len"] + "m</td><td>" + json["loop"] + "</td></tr>");
                    }
                }
                alert("Displayed path found between for Routes and Tracks ");
            },
            error: function(error) {
                if (error.status == 450) {
                    alert("Error, latitudes must be such that '-90.0 <= value <= 90.0'");
                } else if (error.status == 451) {
                    alert("Error, longitudes must be such that '-180.0 <= value < 180.0'");
                } else if (error.status == 452) {
                    alert("Error, delta must be such that 'value >= 0'");
                } else {
                    alert("Error");
                }
            }
        });
    });

    /* Database */
    $('#refresh').prop('disabled', true);
    $('#store').prop('disabled', true);
    $('#clear').prop('disabled', true);
    queriesDisable();

    /***********************
     * CODE FOR FIND PATHS *
     **********************/
    /* client side code for logging in */
    $('#login').on('click', function() {
        let username = $("#username").val();
		let password = $("#password").val();
		let dbname = $("#dbname").val();

        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/dblogin',
            data: {
                username: username,
                password: password,
                dbname: dbname,
            },

            success: function(data) {
                let json = data;
                $("#login").html('Logged In');
                $("#login").prop('disabled', true);
                $('#refresh').prop('disabled', false);
                if (json["regfiles"] != 0) {
                    $('#store').prop('disabled', false);
                }
                if (json["tablefiles"] != 0) {
                    $('#clear').prop('disabled', false);
                    queriesEnable();
                }
            
                loggedin = true;
                console.log("Signed in!\n");
                alert('Successfully Signed In');
                $.ajax({
                    type: 'get',
                    dataType: 'json',
                    url: '/dbrefresh',
        
                    success: function (data) {
                        alert("Database has " + data[0] + " files, " + data[1] + " routes and " + data[2] + " points.")
                    },
                    fail: function(error) {
                        console.log("Error: " + error.status + " occurred")
                        alert("Error occurred");
                    }
                });
            },
            error: function(error) {
                alert("Please check your credentials, and attempt to login again");
            }
        });
    });

    /* client side code refreshing all database data */
    $('#refresh').on('click', function() {
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/dbrefresh',

            success: function (data) {
                alert("Database has " + data[0] + " files, " + data[1] + " routes and " + data[2] + " points.")
            },
            fail: function(error) {
                console.log("Error: " + error.status + " occurred")
                alert("Error occurred");
            }
        });
    });

    /* client side code for storing all database data */
    $('#store').click(function() {    
        $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/dbstore',

            success: function (data) {
                let json = data;
                if (json["tablefiles"] != 0) {
                    $('#clear').prop('disabled', false);
                    queriesEnable();
                }
                alert("All files stored in database");
                $.ajax({
                    type: 'get',
                    dataType: 'json',
                    url: '/dbrefresh',
        
                    success: function (data) {
                        alert("Database has " + data[0] + " files, " + data[1] + " routes and " + data[2] + " points.")
                    },
                    fail: function(error) {
                        console.log("Error: " + error.status + " occurred")
                        alert("Error occurred");
                    }
                });
            },
            error: function(error) {
                console.log("Error: " + error.status + " occurred")
                alert("Error occurred");
            }
        });
    });

    /* client side code deleting all database data */
    $('#clear').on('click', function() {
        $.ajax({
            type: 'get',
            url: '/dbcleardata',

            success: function(data) {
                $('#clear').prop('disabled', true);
                queriesDisable();
                alert("All database data has been cleared");
                $.ajax({
                    type: 'get',
                    dataType: 'json',
                    url: '/dbrefresh',
        
                    success: function (data) {
                        alert("Database has " + data[0] + " files, " + data[1] + " routes and " + data[2] + " points.")
                    },
                    fail: function(error) {
                        console.log("Error: " + error.status + " occurred")
                        alert("Error occurred");
                    }
                });
            },
            error: function(error) {
                console.log("Error: " + error.status + " occurred")
                alert("Error occurred");
            }
        });
    });

    /* client side code for executing a route query */
    $('#route-execute').on('click', function() {
        let filename = $("#route-query-dropdown-menu").val();;
        let which = "route_name";
        if ($('#route-button-name').prop("checked")) {
            which = "route_name";
        } else if ($('#route-button-length').prop("checked")) {
            which = "route_len";
        }
        
        $.ajax({
            type: 'get',
            url: '/dbqueryroute',
            data: {
                filename: filename,
                which: which,
            },

            success: function(data) {
                $('#route-query-table tbody tr').remove();
                for(let i = 0; i < data.length; i++) {
                    $('#route-query-table').append("<tr><td>" + data[i]["route_name"] + "</td><td>" + data[i]["route_len"] + "</td>");
                }
                alert("Query completed");
            },
            error: function(error) {
                console.log("Error: " + error.status + " occurred")
                alert("Error occurred");
            }
        });
    });

    /* client side code for executing a point query */
    $('#point-execute').on('click', function() {
        let filename = $("#point-query-dropdown-menu").val();
        let rtename = $("#point-route-name").val();
        let which = "route_name";
        if ($('#point-button-name').prop("checked")) {
            which = "route_name";
        } else if ($('#point-button-length').prop("checked")) {
            which = "route_len";
        }

        $.ajax({
            type: 'get',
            url: '/dbquerypoint',
            data: {
                filename: filename,
                which: which,
                rtename: rtename,
            },

            success: function(data) {
                $('#point-query-table tbody tr').remove();
                for(let i = 0; i < data.length; i++) {
                    $('#point-query-table').append("<tr><td>" + data[i]["route_name"] + "</td><td>" + data[i]["point_index"] + "</td><td>" + data[i]["point_name"] + "</td><td>" + data[i]["latitude"] + "</td><td>" + data[i]["longitude"] + "</td>");
                }
                alert("Query completed");
            },
            error: function(error) {
                console.log("Error: " + error.status + " occurred")
                if (error.status == 451) {
                    alert("Please select a file from the drop down menu")
                } else {
                    alert("Error occurred");
                }
            }
        });
    });

    /* client side code for doing a query on shortest/longest routes d*/
    $('#shortest-longest-execute').on('click', function() {
        let filename = $("#shortest-longest-query-dropdown-menu").val();
        let length = $("#shortest-longest-length").val();
        let shortestLongest = "short";
        if ($('#shortest-longest-button-short').prop("checked")) {
            shortestLongest = "short";
        } else if ($('#shortest-longest-button-long').prop("checked")) {
            shortestLongest = "long";
        }
        let which = "route_name";
        if ($('#shortest-longest-button-name').prop("checked")) {
            which = "route_name";
        } else if ($('#shortest-longest-button-length').prop("checked")) {
            which = "route_len";
        }

        $.ajax({
            type: 'get',
            url: '/dbquerylength',
            data: {
                filename: filename,
                length: length,
                shortestLongest: shortestLongest,
                which: which,
            },

            success: function(data) {
                $('#shortest-longest-query-table tbody tr').remove();
                for(let i = 0; i < data.length; i++) {
                    $('#shortest-longest-query-table').append("<tr><td>" + data[i]["file_name"] + "</td><td>" + data[i]["route_name"] + "</td><td>" + data[i]["route_len"] + "</td>");
                }
                alert("Query completed");
            },
            error: function(error) {
                console.log("Error: " + error.status + " occurred")
                if (error.status == 451) {
                    alert("Please enter an integer N value of 1 or greater")
                } else if (error.status == 452) {
                    alert("Please select a file from the drop down menu");
                } else {
                    alert("Error occurred");
                }
            }
        });
    });

    async function queriesEnable() {
        $('#point-query-dropdown-menu').prop('disabled', false);
        $('#point-route-name').prop('disabled', false);
        $('#point-execute').prop('disabled', false);
        $('#point-button-name').prop('disabled', false);
        $('#point-button-length').prop('disabled', false);
        $('#route-query-dropdown-menu').prop('disabled', false);
        $('#route-button-name').prop('disabled', false);
        $('#route-button-length').prop('disabled', false);
        $('#route-execute').prop('disabled', false);
        $('#shortest-longest-query-dropdown-menu').prop('disabled', false);
        $('#shortest-longest-length').prop('disabled', false);
        $('#shortest-longest-button-short').prop('disabled', false);
        $('#shortest-longest-button-long').prop('disabled', false);
        $('#shortest-longest-button-name').prop('disabled', false);
        $('#shortest-longest-button-length').prop('disabled', false);
        $('#shortest-longest-execute').prop('disabled', false);
    }

    async function queriesDisable() {
        $('#point-query-dropdown-menu').prop('disabled', true);
        $('#point-route-name').prop('disabled', true);
        $('#point-execute').prop('disabled', true);
        $('#point-button-name').prop('disabled', true);
        $('#point-button-length').prop('disabled', true);
        $('#route-query-dropdown-menu').prop('disabled', true);
        $('#route-button-name').prop('disabled', true);
        $('#route-button-length').prop('disabled', true);
        $('#route-execute').prop('disabled', true);
        $('#shortest-longest-query-dropdown-menu').prop('disabled', true);
        $('#shortest-longest-length').prop('disabled', true);
        $('#shortest-longest-button-short').prop('disabled', true);
        $('#shortest-longest-button-long').prop('disabled', true);
        $('#shortest-longest-button-name').prop('disabled', true);
        $('#shortest-longest-button-length').prop('disabled', true);
        $('#shortest-longest-execute').prop('disabled', true);
    }
});