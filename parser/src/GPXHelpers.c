#include "GPXParser.h"
#include "GPXHelpers.h"

float getDistVal(double val1, double val2, double val3, double val4) {
    double dx;
    double dy;
    double dz;

	val2 -= val4;
	val2 *= (M_PI/180);
    val1 *= (M_PI/180);
    val3 *= (M_PI/180);
 
	dz = sin(val1) - sin(val3);
	dx = cos(val2) * cos(val1) - cos(val3);
	dy = sin(val2) * cos(val1);
	return (asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * 6371);
}

void deleteDummy(void* data) {
	return;
}

char *validUploadedFile(char *fileName) {
	char *directory = malloc(sizeof(char) * 4096);
	strcpy(directory, "uploads/");
	strcat(directory, fileName);
	char *schema = malloc(sizeof(char) * 4096);
	strcpy(schema, "gpx.xsd");
	GPXdoc *gdoc = createValidGPXdoc(directory, schema);
	if (gdoc == NULL) {
		deleteGPXdoc(gdoc);
		free(directory);
		free(schema);
		return ("File is invalid!");
	} else {
		deleteGPXdoc(gdoc);
		free(directory);
		free(schema);
		return ("File is valid!");
	}
}

/* Checks if gpx file is valid, if it is, return it as a json string */
char *gpxToHTML(char *fileName) {
	char *directory = malloc(sizeof(char) * 4096);
	strcpy(directory, "uploads/");
	strcat(directory, fileName);
	char *schema = malloc(sizeof(char) * 4096);
	strcpy(schema, "gpx.xsd");
	GPXdoc *gdoc = createValidGPXdoc(directory, schema);
	if (gdoc == NULL) {
		deleteGPXdoc(gdoc);
		free(directory);
		free(schema);
		return ("File is invalid!");
	}

	char *retString = GPXtoJSON(gdoc);
	deleteGPXdoc(gdoc);
	free(directory);
	free(schema);
	return (retString);
}

/* Checks if gpx file is valid, if it is, return its routes as a json string */
char *rteToHTML(char *fileName) {
	char *directory = malloc(sizeof(char) * 4096);
	strcpy(directory, "uploads/");
	strcat(directory, fileName);
	char *schema = malloc(sizeof(char) * 4096);
	strcpy(schema, "gpx.xsd");
	GPXdoc *gdoc = createValidGPXdoc(directory, schema);
	if (gdoc == NULL) {
		deleteGPXdoc(gdoc);
		free(directory);
		free(schema);
		return ("File is invalid!");
	}

	char *retString = routeListToJSON(gdoc->routes);
	deleteGPXdoc(gdoc);
	free(directory);
	free(schema);	
	return (retString);
}

/* Checks if gpx file is valid, if it is, return its tracks as a json string */
char *trkToHTML(char *fileName) {
	char *directory = malloc(sizeof(char) * 4096);
	strcpy(directory, "uploads/");
	strcat(directory, fileName);
	char *schema = malloc(sizeof(char) * 4096);
	strcpy(schema, "gpx.xsd");
	GPXdoc *gdoc = createValidGPXdoc(directory, schema);
	if (gdoc == NULL) {
		deleteGPXdoc(gdoc);
		free(directory);
		free(schema);
		return ("File is invalid!");
	}

	char *retString = trackListToJSON(gdoc->tracks);
	deleteGPXdoc(gdoc);
	free(directory);
	free(schema);
	return (retString);
}

/* Convert one waypoint to JSON */
char *ptsToJSON(const Waypoint *wpt, int index) {
    char *theString = malloc(sizeof(char) * 8192);
    if (wpt == NULL) {
        strcpy(theString, "{}");
        return (theString);
    }
	int waypointIndex = index;
    char waypointName[1024] = "";
	double waypointLat = wpt->latitude;
	double waypointLon = wpt->longitude;
	strcpy(waypointName, wpt->name);

    sprintf(theString, "{\"point_index\":\"%d\",\"point_name\":\"%s\",\"latitude\":\"%f\",\"longitude\":\"%f\"}" , waypointIndex, waypointName, waypointLat, waypointLon);
    return(theString);
}

/* Convert all points to HTML */
char *ptsToHTML(char *fileName, int location) {
	char *directory = malloc(sizeof(char) * 4096);
	strcpy(directory, "uploads/");
	strcat(directory, fileName);
	char *schema = malloc(sizeof(char) * 4096);
	strcpy(schema, "gpx.xsd");
	GPXdoc *gdoc = createValidGPXdoc(directory, schema);
	if (gdoc == NULL) {
		deleteGPXdoc(gdoc);
		free(directory);
		free(schema);
		return ("File is invalid!");
	}

	int counter = 0;
	char *retString;
    void *elem;
    ListIterator iter = createIterator((List *)gdoc->routes);
    while ((elem = nextElement(&iter)) != NULL) {
        Route* tmpRteData = (Route *)elem;
		if (counter == location) {
			retString = ptsListToJSON(tmpRteData->waypoints, location);
			break;
		}
        counter++;
    }
	deleteGPXdoc(gdoc);
	free(directory);
	free(schema);
	return (retString);
}

/* Convert list of waypoints to JSON */
char *ptsListToJSON(const List *list, int location) {
    char *theString = malloc(sizeof(char) * 8192);
    strcpy(theString, "");
    if (list == NULL) {
        strcpy(theString, "[]");
        return (theString);
    } else if (getLength((List *)list) == 0) {
        strcpy(theString, "[]");
        return (theString);  
    }

    int counter = 0;
    sprintf(theString + strlen(theString), "[");

    void *elem;
    ListIterator iter = createIterator((List *)list);
    while ((elem = nextElement(&iter)) != NULL) {
        Waypoint* tmpWptData = (Waypoint *)elem;
        char *tempString = ptsToJSON(tmpWptData, counter);
        sprintf(theString + strlen(theString), "%s", tempString);
        free(tempString);
        if (counter != getLength((List *)list)-1) {
            sprintf(theString + strlen(theString), ",");
        }
        counter++;
    }
    sprintf(theString + strlen(theString), "]");
    return(theString);
}

/* Convert otherData to usable content for a JSON string */
char* otherDataToJSON(const GPXData *od) {
    char *theString = malloc(sizeof(char) * 8192);
    if (od == NULL) {
        strcpy(theString, "{}");
        return (theString);
    }

    char otherDataName[1024] = "";
	char otherDataContents[1024] = "";
	strcpy(otherDataName, od->name);
	strcpy(otherDataContents, od->value);

    sprintf(theString, "%s -> %s", otherDataName, otherDataContents);
    return(theString);
}

/* Convert otherData list to a full JSON string */
char *otherDataListToJSON(const List *list) {
	char *theString = malloc(sizeof(char) * 8192);
    strcpy(theString, "");
    if (list == NULL) {
        strcpy(theString, "{}");
        return (theString);
    } else if (getLength((List *)list) == 0) {
        strcpy(theString, "{}");
        return (theString);  
    }

    int counter = 0;
    sprintf(theString + strlen(theString), "{\"otherData\":\"");

    void *elem;
    ListIterator iter = createIterator((List *)list);
    while ((elem = nextElement(&iter)) != NULL) {
        GPXData* tmpGPXData = (GPXData *)elem;
        char *tempString = otherDataToJSON(tmpGPXData);
        sprintf(theString + strlen(theString), "%s", tempString);
        free(tempString);
        sprintf(theString + strlen(theString), "\\n");
        counter++;
    }
    sprintf(theString + strlen(theString), "\"}");
	
    return(theString);
}

/* Find the row and get the data */
char *otherDataToHTML(char *fileName, int index) {
	char *directory = malloc(sizeof(char) * 4096);
	strcpy(directory, "uploads/");
	strcat(directory, fileName);
	char *schema = malloc(sizeof(char) * 4096);
	strcpy(schema, "gpx.xsd");
	GPXdoc *gdoc = createValidGPXdoc(directory, schema);
	if (gdoc == NULL) {
		deleteGPXdoc(gdoc);
		free(directory);
		free(schema);
		return ("File is invalid!");
	}

	char *retString = malloc(sizeof(char) * 4096);
	strcpy(retString, "No GPXData");
	
	int counter = 1;
	void *elem;
    ListIterator iter = createIterator(gdoc->routes);
    while ((elem = nextElement(&iter)) != NULL) {
        Route* tmpRouteData = (Route *)elem;
		if (counter == index) {
			retString = otherDataListToJSON(tmpRouteData->otherData);
		}
        counter++;
    }

    iter = createIterator(gdoc->tracks);
    while ((elem = nextElement(&iter)) != NULL) {
        Track* tmpTrackData = (Track *)elem;
		if (counter == index) {
			retString = otherDataListToJSON(tmpTrackData->otherData);
		}
        counter++;
    }
	deleteGPXdoc(gdoc);
	free(directory);
	free(schema);	
	return (retString);
}

/* Function that changes a route or track name */
char *changeRouteOrTrack(char *fileName, char *newName, int index) {
	char *directory = malloc(sizeof(char) * 4096);
	strcpy(directory, "uploads/");
	strcat(directory, fileName);
	char *schema = malloc(sizeof(char) * 4096);
	strcpy(schema, "gpx.xsd");
	GPXdoc *gdoc = createValidGPXdoc(directory, schema);
	if (gdoc == NULL) {
		deleteGPXdoc(gdoc);
		free(directory);
		free(schema);
		return ("File is invalid!");
	}

	char *retString = malloc(sizeof(char) * 4096);
	strcpy(retString, "Row not Found");
	
	int counter = 1;
	void *elem;
    ListIterator iter = createIterator(gdoc->routes);
    while ((elem = nextElement(&iter)) != NULL) {
        Route* tmpRouteData = (Route *)elem;
		if (counter == index) {
			strcpy(tmpRouteData->name, newName);
			strcpy(retString, "Changed name");
		}
        counter++;
    }

    iter = createIterator(gdoc->tracks);
    while ((elem = nextElement(&iter)) != NULL) {
        Track* tmpTrackData = (Track *)elem;
		if (counter == index) {
			strcpy(tmpTrackData->name, newName);
			strcpy(retString, "Changed name");
		}
        counter++;
    }
	
	bool isvalid = validateGPXDoc(gdoc, schema);
	if (isvalid == false) {
		deleteGPXdoc(gdoc);
		free(directory);
		free(schema);
		return ("File is invalid!");
	}

	writeGPXdoc(gdoc, directory);
	if (gdoc == NULL) {
		strcpy(retString, "File is invalid!");
	}
	deleteGPXdoc(gdoc);
	free(directory);
	free(schema);	
	return (retString);
}

/* Add a new gpx file to uploads folder */
char *addGPXToUploads(char *stringJSON, char *fileName) {
	char *retString = malloc(sizeof(char) * 4096);
	strcpy(retString, "Valid");
	GPXdoc *gdoc = JSONtoGPX(stringJSON);
	if (gdoc == NULL) {
		strcpy(retString, "Error");
	}
	writeGPXdoc(gdoc, fileName);
	if (gdoc == NULL) {
		strcpy(retString, "Error");
	}
	return(retString);
}

/* Function that adds a route to a file */
char *addRouteToFile(char *fileName, char *waypointsJSON, char *routeName) {
	char *retString = malloc(sizeof(char) * 4096);
	strcpy(retString, "Error occurred");

	/* Opening a GPXDoc */
	char *directory = malloc(sizeof(char) * 4096);
	strcpy(directory, "uploads/");
	strcat(directory, fileName);
	char *schema = malloc(sizeof(char) * 4096);
	strcpy(schema, "gpx.xsd");
	GPXdoc *gdoc = createValidGPXdoc(directory, schema);
	if (gdoc == NULL) {
		deleteGPXdoc(gdoc);
		free(directory);
		free(schema);
		return ("File is invalid!");
	}
	Route *rt = JSONtoRoute(routeName);
	addRoute(gdoc, rt);
   
    /* Get first token, then go through all other tokens */
	if (waypointsJSON != NULL) {

		/* Modifying JSON string to be usable with module 2 functions */
		waypointsJSON = replaceSubString(waypointsJSON, "\",\"", "|");
		waypointsJSON = replaceSubString(waypointsJSON, "[\"", "");
		waypointsJSON = replaceSubString(waypointsJSON, "\"]", "");
		waypointsJSON = replaceSubString(waypointsJSON, "\\", "");

		char *token;
		char tokens[1024][128];
		int counter = 0;
		token = strtok(waypointsJSON, "|");
		if (token == NULL) {
			return("No waypoints");
		}
		strcpy(tokens[counter++], token);

		/* Walk through other tokens */
		while (token != NULL) {
			token = strtok(NULL, "|");
			if (token == NULL) {
				break;
			}
			strcpy(tokens[counter++], token);
		}

		/* Creating waypoints with the data */
		for (int i = 0; i < counter; i++) {
			Waypoint *tmpWaypoint = JSONtoWaypoint(tokens[i]);
			addWaypoint(rt, tmpWaypoint);
		}
	}

	writeGPXdoc(gdoc, fileName);
	if (gdoc == NULL) {
		strcpy(retString, "Error");
	}
	return ("retString");
}

/* Function that replaces a substring with another substring */
char *replaceSubString(char *string, char *toReplace, char *replacement) {
    char buffer[strlen(string)];
    memset(buffer, 0, sizeof(buffer));
    for (int i = 0; i < strlen(string); i++){
        if (!strncmp(string + i, toReplace, strlen(toReplace))){
            strcat(buffer ,replacement);
            i += strlen(toReplace) - 1;
        } else {
			strncat(buffer, string + i, 1);
		}
    }

    strcpy(string, buffer);
    return (string);
}

/* Function that finds the number of paths that fit the delta value for routes */
char *findRoutePathsToJSON(char *fileArray, float latStart, float lonStart, float latEnd, float lonEnd, float delta) {
	if (fileArray == NULL) {
		return("No files");
	}

	/* Opening a GPXDoc */
	char *directory = malloc(sizeof(char) * 4096);
	char *schema = malloc(sizeof(char) * 4096);
	strcpy(schema, "gpx.xsd");

	/* Setting up tokens */
	fileArray = replaceSubString(fileArray, "\",\"", "|");
	fileArray = replaceSubString(fileArray, "[\"", "");
	fileArray = replaceSubString(fileArray, "\"]", "");
	char *token;
	char tokens[1024][128];
	int counter = 0;
	token = strtok(fileArray, "|");
	if (token == NULL) {
		return("No waypoints");
	}
	strcpy(tokens[counter++], token);

	/* Walk through other tokens */
	while (token != NULL) {
		token = strtok(NULL, "|");
		if (token == NULL) {
			break;
		}
		strcpy(tokens[counter++], token);
	}

	/* Getting route lists that fit the request */
	List *routeList = initializeList(&routeToString, &deleteRoute, &compareRoutes);
	if (counter != 0) {
		for (int i = 0; i < counter; i++) {
			/* Getting the list from the specific file ready */
			strcpy(directory, "");
			strcpy(directory, "uploads/");
			strcat(directory, tokens[i]);
			GPXdoc *gdoc = createValidGPXdoc(directory, schema);
			List *tempRouteList = getRoutesBetween(gdoc, latStart, lonStart, latEnd, lonEnd, delta);
			if (tempRouteList != NULL) {
				/* Adding all routes to the whole list */
				void *elem;
				ListIterator iter = createIterator(tempRouteList);
				while ((elem = nextElement(&iter)) != NULL) {
					Route* tmpRouteData = (Route *)elem;
					insertBack(routeList, tmpRouteData);
				}
			}
		}
	}

	char *routeListJSON = routeListToJSON(routeList);
	return(routeListJSON);
}

/* Function that finds the number of paths that fit the delta value for routes */
char *findTrackPathsToJSON(char *fileArray, float latStart, float lonStart, float latEnd, float lonEnd, float delta) {
	if (fileArray == NULL) {
		return("No files");
	}

	/* Opening a GPXDoc */
	char *directory = malloc(sizeof(char) * 4096);
	char *schema = malloc(sizeof(char) * 4096);
	strcpy(schema, "gpx.xsd");

	/* Setting up tokens */
	fileArray = replaceSubString(fileArray, "\",\"", "|");
	fileArray = replaceSubString(fileArray, "[\"", "");
	fileArray = replaceSubString(fileArray, "\"]", "");
	char *token;
	char tokens[1024][128];
	int counter = 0;
	token = strtok(fileArray, "|");
	if (token == NULL) {
		return("No waypoints");
	}
	strcpy(tokens[counter++], token);

	/* Walk through other tokens */
	while (token != NULL) {
		token = strtok(NULL, "|");
		if (token == NULL) {
			break;
		}
		strcpy(tokens[counter++], token);
	}

	/* Getting track lists that fit the request */
	List *trackList = initializeList(&trackToString, &deleteTrack, &compareTracks);
	if (counter != 0) {
		for (int i = 0; i < counter; i++) {
			/* Getting the list from the specific file ready */
			strcpy(directory, "");
			strcpy(directory, "uploads/");
			strcat(directory, tokens[i]);
			GPXdoc *gdoc = createValidGPXdoc(directory, schema);
			List *tempTrackList = getTracksBetween(gdoc, latStart, lonStart, latEnd, lonEnd, delta);
			if (tempTrackList != NULL) {
				/* Adding all tracks to the whole list */
				void *elem;
				ListIterator iter = createIterator(tempTrackList);
				while ((elem = nextElement(&iter)) != NULL) {
					Track* tmpTrackData = (Track *)elem;
					insertBack(trackList, tmpTrackData);
				}
			}
		}
	}

	char *trackListJSON = trackListToJSON(trackList);
	return(trackListJSON);
}