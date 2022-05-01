#include "GPXParser.h"
#include "GPXHelpers.h"

GPXdoc* createGPXdoc(char* fileName) {
    /* Checking if the fileName is NULL or an empty string, if it is then return NULL */
    if (fileName == NULL) {
        return(NULL);
    } else if (strlen(fileName) == 0) {
        return(NULL);
    }

    GPXdoc *gdoc = NULL;
    gdoc = (GPXdoc*)malloc(sizeof(GPXdoc));

    xmlDoc *doc = NULL;
    xmlNode *rootElement = NULL;
    xmlNode *curNode = NULL;

    xmlNode *wptNode = NULL;
    xmlNode *wptNodeChild = NULL;
    xmlNode *rteptNode = NULL;
    xmlNode *rteptNodeChild = NULL;
    xmlNode *trkNode = NULL;
    xmlNode *trkNodeChild = NULL;

    xmlAttr *attr = NULL;

    int foundWaypointName = 0;
    int foundRouteName = 0;
    int foundTrackName = 0;

    LIBXML_TEST_VERSION

    /* Parse the file and get the DOM */
    doc = xmlReadFile(fileName, NULL, 0);
    if (doc == NULL) {
        free(gdoc);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return(NULL);
    }

    rootElement = xmlDocGetRootElement(doc);
    curNode = rootElement;
    char *rootName = (char*)rootElement -> name;

    /*************************
    * GETTING BASIC GPX DATA *
    **************************/
    gdoc -> creator = malloc(sizeof(char) * 8196);
    /* Check for the namespace */
    if (strcmp(rootName, "gpx") == 0) {
        /* Setting the namespace */
        char *namespaceName = (char*)(rootElement -> ns -> href);
        strcpy(gdoc -> namespace, namespaceName);
    }


    for (attr = curNode -> properties; attr != NULL; attr = attr -> next) {
        /* Cycling through all the attributes of the GPX node */
        xmlNode *value = attr -> children;
        char *attrName = (char*)attr -> name;
        char *cont = (char*)(value -> content);

        /* Checking for the version attribute and setting the version */
        if (strcmp(attrName, "version") == 0) {
            gdoc -> version = atof(cont);
        }

        /* Checking for the creator attribute and setting the creator */
        if (strcmp(attrName, "creator") == 0) {
            /* Checking if the content is NULL or empty */
            strcpy(gdoc -> creator, cont);
        }
    }

    /*****************************
    * GETTING WAYPOINTS GPX DATA *
    *****************************/
    List *waypointList = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    /* Going through all of the children */
    for (wptNode = rootElement -> children; wptNode != NULL; wptNode = wptNode -> next) {
        if (wptNode -> type == XML_ELEMENT_NODE) {
            char *wptNodeName = (char*)wptNode -> name;

            /* Checking if the child is a waypoint */
            if (strcmp(wptNodeName, "wpt") == 0) {
                /* A WAYPOINT HAS BEEN ENCOUNTERED */
                Waypoint *tmpWaypoint;
                tmpWaypoint = (Waypoint*)malloc(sizeof(Waypoint));

                /* Going through waypoint attributes */
                for (attr = wptNode -> properties; attr != NULL; attr = attr -> next) {
                    /* Cycling through all the attributes of the GPX node */
                    xmlNode *value = attr -> children;
                    char *attrName = (char*)attr -> name;
                    char *cont = (char*)(value -> content);

                    /* Checking if the content of lat is NULL or empty */
                    if (strcmp(attrName, "lat") == 0) {
                        tmpWaypoint -> latitude = atof(cont);
                    }

                    /* Checking if the content of lon is NULL or empty */
                    if (strcmp(attrName, "lon") == 0) {
                        tmpWaypoint -> longitude = atof(cont);
                    }
                }

                /* Going through the waypoint children */
                List *otherDataList = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                for (wptNodeChild = wptNode -> children; wptNodeChild != NULL; wptNodeChild = wptNodeChild -> next) {
                    if (wptNodeChild -> type == XML_ELEMENT_NODE) {
                        char *wptNodeChildName = (char*)wptNodeChild -> name;
                        char *xmlNodeContent = (char*)xmlNodeGetContent(wptNodeChild);

                        /* Checking for name data or other data */
                        if (strcmp(wptNodeChildName, "name") == 0) {
                            /* Setting the waypoint name */
                            char *nameDyn = malloc(sizeof(char) * 8196);
                            strcpy(nameDyn, xmlNodeContent);
                            foundWaypointName = 1;
                            tmpWaypoint -> name = nameDyn;
                        } else {
                            /* Making a GPXData struct to be stored in the otherData list */
                            GPXData *tmpGPXData;
                            tmpGPXData = (GPXData*)malloc(sizeof(GPXData) + 8196 * sizeof(char));
                            strcpy(tmpGPXData -> name, wptNodeChildName);
                            strcpy(tmpGPXData -> value, xmlNodeContent);
                            insertBack(otherDataList, tmpGPXData);
                        }
                        xmlFree(xmlNodeContent);
                    }
                }

                /* Set the waypoint's other data */
                tmpWaypoint -> otherData = otherDataList;

                /* If the waypoint name wasn't found, set it to a space */
                if (foundWaypointName != 1) {
                    char *nameDyn = malloc(sizeof(char) * 8196);
                    strcpy(nameDyn, "");
                    tmpWaypoint -> name = nameDyn;
                }
                foundWaypointName = 0;

                /* Add the waypoint to the waypoint list */
                insertBack(waypointList, tmpWaypoint);
            }
        }
    }
    gdoc -> waypoints = waypointList;

    /**************************
    * GETTING ROUTES GPX DATA *
    **************************/
    List *routeList = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    for (curNode = rootElement -> children; curNode != NULL; curNode = curNode -> next) {
        if (curNode -> type == XML_ELEMENT_NODE) {
            char *curNodeName = (char*)curNode -> name;

            /* Checking if the child is a route */
            if (strcmp(curNodeName, "rte") == 0) {
                /* A ROUTE HAS BEEN ENCOUNTERED */
                Route *tmpRoute;
                tmpRoute = (Route*)malloc(sizeof(Route));

                /* Going through the route children */
                List *routeOtherDataList = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                List *routeWaypointList = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
                for (rteptNode = curNode -> children; rteptNode != NULL; rteptNode = rteptNode -> next) {
                    if (rteptNode -> type == XML_ELEMENT_NODE) {
                        char *rteptNodeName = (char*)rteptNode -> name;
                        char *xmlNodeContent = (char*)xmlNodeGetContent(rteptNode);

                        /* Checking for name data or other data */
                        if (strcmp(rteptNodeName, "name") == 0) {
                            /* Setting the route name */
                            char *nameDyn = malloc(sizeof(char) * 8196);
                            strcpy(nameDyn, xmlNodeContent);
                            foundRouteName = 1;
                            tmpRoute -> name = nameDyn;
                        } else if (strcmp(rteptNodeName, "rtept") == 0) {
                            /* A WAYPOINT HAS BEEN ENCOUNTERED */
                            Waypoint *tmpWaypoint;
                            tmpWaypoint = (Waypoint*)malloc(sizeof(Waypoint));

                            /* Going through waypoint attributes */
                            for (attr = rteptNode -> properties; attr != NULL; attr = attr -> next) {
                                /* Cycling through all the attributes of the GPX node */
                                xmlNode *value = attr -> children;
                                char *attrName = (char*)attr -> name;
                                char *cont = (char*)(value -> content);

                                /* Checking if the content of lat */
                                if (strcmp(attrName, "lat") == 0) {
                                    tmpWaypoint -> latitude = atof(cont);
                                }

                                /* Checking if the content of lon */
                                if (strcmp(attrName, "lon") == 0) {
                                    tmpWaypoint -> longitude = atof(cont);
                                }
                            }

                            /* Going through the waypoint children */
                            List *otherDataList = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                            for (rteptNodeChild = rteptNode -> children; rteptNodeChild != NULL; rteptNodeChild = rteptNodeChild -> next) {
                                if (rteptNodeChild -> type == XML_ELEMENT_NODE) {
                                    char *rteptNodeChildName = (char*)rteptNodeChild -> name;
                                    char *xmlNodeContentChild = (char*)xmlNodeGetContent(rteptNodeChild);

                                    /* Checking for name data or other data */
                                    if (strcmp(rteptNodeChildName, "name") == 0) {
                                        /* Setting the waypoint name */
                                        char *nameDyn = malloc(sizeof(char) * 8196);
                                        strcpy(nameDyn, xmlNodeContentChild);
                                        foundWaypointName = 1;
                                        tmpWaypoint -> name = nameDyn;
                                    } else {
                                        /* Getting otherData for waypoint */
                                        GPXData *tmpGPXData;
                                        tmpGPXData = (GPXData*)malloc(sizeof(GPXData) + 8196 * sizeof(char));
                                        strcpy(tmpGPXData -> name, rteptNodeChildName);
                                        strcpy(tmpGPXData -> value, xmlNodeContentChild);
                                        insertBack(otherDataList, tmpGPXData);
                                    }
                                    xmlFree(xmlNodeContentChild);
                                }
                            }
                            tmpWaypoint -> otherData = otherDataList;

                            /* If the waypoint name wasn't found, set it to a space */
                            if (foundWaypointName != 1) {
                                char *nameDyn = malloc(sizeof(char) * 8196);
                                strcpy(nameDyn, "");
                                tmpWaypoint -> name = nameDyn;
                            }
                            foundWaypointName = 0;

                            /* Add the waypoint to the waypoint list */
                            insertBack(routeWaypointList, tmpWaypoint);
                        } else {
                            /* Getting otherData for route */
                            GPXData *tmpGPXData;
                            tmpGPXData = (GPXData*)malloc(sizeof(GPXData) + 8196 * sizeof(char));
                            strcpy(tmpGPXData -> name, rteptNodeName);
                            strcpy(tmpGPXData -> value, xmlNodeContent);
                            insertBack(routeOtherDataList, tmpGPXData);
                        }
                        xmlFree(xmlNodeContent);
                    }
                }
                tmpRoute -> waypoints = routeWaypointList;
                tmpRoute -> otherData = routeOtherDataList;

                /* If the route name wasn't found, set it to a space */
                if (foundRouteName != 1) {
                    char *nameDyn = malloc(sizeof(char) * 8196);
                    strcpy(nameDyn, "");
                    tmpRoute -> name = nameDyn;
                }
                foundRouteName = 0;

                /* Add the route to the route list */
                insertBack(routeList, tmpRoute);
            }
        }
    }
    gdoc -> routes = routeList;


    /**************************
    * GETTING TRACKS GPX DATA *
    **************************/
    List *trackList = initializeList(&trackToString, &deleteTrack, &compareTracks);
    for (curNode = rootElement -> children; curNode != NULL; curNode = curNode -> next) {
        if (curNode -> type == XML_ELEMENT_NODE) {
            char *curNodeName = (char*)curNode -> name;

            /* Checking if the child is a track */
            if (strcmp(curNodeName, "trk") == 0) {
                /* A TRACK HAS BEEN FOUND */
                Track *tmpTrack;
                tmpTrack = (Track*)malloc(sizeof(Track));

                /* Going through the track children */
                List *trackOtherDataList = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                List *segmentList = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
                for (trkNode = curNode -> children; trkNode != NULL; trkNode = trkNode -> next) {
                    if (trkNode -> type == XML_ELEMENT_NODE) {
                        char *trkNodeName = (char*)trkNode -> name;
                        char *xmlNodeContent = (char*)xmlNodeGetContent(trkNode);

                        /* Checking for name data or other data */
                        if (strcmp(trkNodeName, "name") == 0) {
                            /* Setting the track name */
                            char *nameDyn = malloc(sizeof(char) * 8196);
                            strcpy(nameDyn, xmlNodeContent);
                            foundTrackName = 1;
                            tmpTrack -> name = nameDyn;
                        } else if (strcmp(trkNodeName, "trkseg") == 0) {
                            /* A TRACK SEGMENT HAS BEEN FOUND */
                            TrackSegment *tmpTrackSegment;
                            tmpTrackSegment = (TrackSegment*)malloc(sizeof(TrackSegment));

                            /* Going through the waypoint children */
                            List *waypointList = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
                            for (trkNodeChild = trkNode -> children; trkNodeChild != NULL; trkNodeChild = trkNodeChild -> next) {
                                if (trkNodeChild -> type == XML_ELEMENT_NODE) {
                                    char *trkNodeChildName = (char*)trkNodeChild -> name;

                                    /* Checking for name data or other data */
                                    if (strcmp(trkNodeChildName, "trkpt") == 0) {
                                        /* A TRACK POINT HAS BEEN FOUND */
                                        Waypoint *tmpWaypoint;
                                        tmpWaypoint = (Waypoint*)malloc(sizeof(Waypoint));

                                        /* Going through waypoint attributes */
                                        for (attr = trkNodeChild -> properties; attr != NULL; attr = attr -> next) {
                                            /* Cycling through all the attributes of the GPX node */
                                            xmlNode *value = attr -> children;
                                            char *attrName = (char*)attr -> name;
                                            char *cont = (char*)(value -> content);

                                            /* Checking if the content of lat is NULL or empty */
                                            if (strcmp(attrName, "lat") == 0) {
                                                tmpWaypoint -> latitude = atof(cont);
                                            }

                                            /* Checking if the content of lon is NULL or empty */
                                            if (strcmp(attrName, "lon") == 0) {
                                                tmpWaypoint -> longitude = atof(cont);
                                            }
                                        }

                                        /* Going through the waypoint children */
                                        List *otherDataList = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
                                        for (wptNodeChild = trkNodeChild -> children; wptNodeChild != NULL; wptNodeChild = wptNodeChild -> next) {
                                            if (wptNodeChild -> type == XML_ELEMENT_NODE) {
                                                char *wptNodeChildName = (char*)wptNodeChild -> name;
                                                char *xmlNodeContentInner = (char*)xmlNodeGetContent(wptNodeChild);

                                                /* Checking for name data or other data */
                                                if (strcmp(wptNodeChildName, "name") == 0) {
                                                    /* Setting the waypoint name */
                                                    char *nameDyn = malloc(sizeof(char) * 8196);
                                                    strcpy(nameDyn, xmlNodeContentInner);
                                                    foundWaypointName = 1;
                                                    tmpWaypoint -> name = nameDyn;
                                                } else {
                                                    /* Making a GPXData struct to be stored in the otherData list */
                                                    GPXData *tmpGPXData;
                                                    tmpGPXData = (GPXData*)malloc(sizeof(GPXData) + 8196 * sizeof(char));
                                                    strcpy(tmpGPXData -> name, wptNodeChildName);
                                                    strcpy(tmpGPXData -> value, xmlNodeContentInner);
                                                    insertBack(otherDataList, tmpGPXData);
                                                }
                                                xmlFree(xmlNodeContentInner);
                                            }
                                        }

                                        /* Set the waypoint's other data */
                                        tmpWaypoint -> otherData = otherDataList;

                                        /* If the waypoint name wasn't found, set it to a space */
                                        if (foundWaypointName != 1) {
                                            char *nameDyn = malloc(sizeof(char) * 8196);
                                            strcpy(nameDyn, "");
                                            tmpWaypoint -> name = nameDyn;
                                        }
                                        foundWaypointName = 0;

                                        /* Add the waypoint to the waypoint list */
                                        insertBack(waypointList, tmpWaypoint);

                                    }
                                }
                            }
                            tmpTrackSegment -> waypoints = waypointList;
                            
                            /* Adding the track segment to the track segment list */
                            insertBack(segmentList, tmpTrackSegment);

                        } else {
                            /* Getting otherData for track */
                            GPXData *tmpGPXData;
                            tmpGPXData = (GPXData*)malloc(sizeof(GPXData) + 8196 * sizeof(char));
                            strcpy(tmpGPXData -> name, trkNodeName);
                            strcpy(tmpGPXData -> value, xmlNodeContent);
                            insertBack(trackOtherDataList, tmpGPXData);
                        }
                        xmlFree(xmlNodeContent);
                    }
                }
                tmpTrack -> segments = segmentList;
                tmpTrack -> otherData = trackOtherDataList;

                /* Return NULL if */
                if (foundTrackName != 1) {
                    char *nameDyn = malloc(sizeof(char) * 8196);
                    strcpy(nameDyn, "");
                    tmpTrack -> name = nameDyn;
                }
                foundTrackName = 0;

                /* Adding the track to the list of tracks */
                insertBack(trackList, tmpTrack);
            }
        }
    }
    gdoc -> tracks = trackList;

    xmlFreeDoc(doc);
    xmlCleanupParser();
    return(gdoc);
}

char* GPXdocToString(GPXdoc* doc) {
    if (doc != NULL) {
        char *theString = malloc(sizeof(char) * 8196);
        void* elem;
        sprintf(theString, "Namespace:\t%s\n", doc -> namespace);
        sprintf(theString + strlen(theString), "Version:\t%f\n", doc -> version);
        sprintf(theString + strlen(theString), "Creator:\t%s\n", doc -> creator);

        ListIterator iter = createIterator(doc -> waypoints);
        while ((elem = nextElement(&iter)) != NULL){
            Waypoint* tmpWaypoint = (Waypoint*)elem;
            char* str = (doc -> waypoints)->printData(tmpWaypoint);
            sprintf(theString + strlen(theString), "%s", str);
            free(str);
        }

        iter = createIterator(doc -> tracks);
        while ((elem = nextElement(&iter)) != NULL){
            Track* tmpTrack = (Track*)elem;
            char* str = (doc -> tracks)->printData(tmpTrack);
            sprintf(theString + strlen(theString), "%s", str);
            free(str);
        }

        return(theString);
    } else {
        return(NULL);
    }
}

void deleteGPXdoc(GPXdoc* doc) {
    if (doc != NULL) {
        free(doc -> creator);
        freeList(doc -> waypoints);
        freeList(doc -> routes);
        freeList(doc -> tracks);
        free(doc);
    }
}

int getNumWaypoints(const GPXdoc* doc) {
    if (doc == NULL) {
        return(0);
    } else {
        int counter = 0;
        void *elem;

        ListIterator iter = createIterator(doc -> waypoints);
        while ((elem = nextElement(&iter)) != NULL) {
            counter++;
        }
        return(counter);
    }
}

int getNumRoutes(const GPXdoc* doc) {
    if (doc == NULL) {
        return(0);
    } else {
        int counter = 0;
        void *elem;

        ListIterator iter = createIterator(doc -> routes);
        while ((elem = nextElement(&iter)) != NULL) {
            counter++;
        }
        return(counter);
    }
}

int getNumTracks(const GPXdoc* doc) {
    if (doc == NULL) {
        return(0);
    } else {
        int counter = 0;
        void *elem;

        ListIterator iter = createIterator(doc -> tracks);
        while ((elem = nextElement(&iter)) != NULL){
            counter++;
        }
        return(counter);
    }
}

int getNumSegments(const GPXdoc* doc) {
    if (doc == NULL) {
        return(0);
    } else {
        int counter = 0;
        void *elem;
        void *elemInner;

        ListIterator iter = createIterator(doc -> tracks);
        while ((elem = nextElement(&iter)) != NULL) {
            Track* tmpData = (Track*)elem;
            ListIterator iterInner = createIterator(tmpData -> segments);
            while ((elemInner = nextElement(&iterInner)) != NULL) {
                counter++;
            }
        }
        return(counter);
    }
}

int getNumGPXData(const GPXdoc* doc) {
    if (doc == NULL) {
        return(0);
    } else {
        int counter = 0;
        void *elem1;
        void *elem2;
        void *elem3;
        void *elem4;
        ListIterator iter1;
        ListIterator iter2;
        ListIterator iter3;
        ListIterator iter4;

        /* Go through all the regular waypoints */
        iter1 = createIterator(doc -> waypoints);
        while ((elem1 = nextElement(&iter1)) != NULL) {
            Waypoint* tmpData1 = (Waypoint*)elem1;

            if(strcmp(tmpData1 -> name, "") != 0) {
                counter++;
            }

            /* Go through every waypoints otherData node */
            iter2 = createIterator(tmpData1 -> otherData);
            while ((elem2 = nextElement(&iter2)) != NULL) {
                counter++;
            }
        }

        /* Go through all the routes */
        iter1 = createIterator(doc -> routes);
        while ((elem1 = nextElement(&iter1)) != NULL) {
            Route* tmpData1 = (Route*)elem1;

            if(strcmp(tmpData1 -> name, "") != 0) {
                counter++;
            }

            /* Go through every waypoint */
            iter2 = createIterator(tmpData1 -> waypoints);
            while ((elem2 = nextElement(&iter2)) != NULL) {
                Waypoint* tmpData2 = (Waypoint*)elem2;

                if(strcmp(tmpData2 -> name, "") != 0) {
                    counter++;
                }

                /* Go through every waypoint otherData node */
                iter3 = createIterator(tmpData2 -> otherData);
                while ((elem3 = nextElement(&iter3)) != NULL) {
                    counter++;
                }
            }

            /* Go through every route otherData node */
            iter2 = createIterator(tmpData1 -> otherData);
            while ((elem2 = nextElement(&iter2)) != NULL) {
                counter++;
            }
        }

        /* Go through all the tracks */
        iter1 = createIterator(doc -> tracks);
        while ((elem1 = nextElement(&iter1)) != NULL) {
            Track* tmpData1 = (Track*)elem1;

            if(strcmp(tmpData1 -> name, "") != 0) {
                counter++;
            }

            /* Go through every waypoint */
            iter2 = createIterator(tmpData1 -> segments);
            while ((elem2 = nextElement(&iter2)) != NULL) {
                TrackSegment* tmpData2 = (TrackSegment*)elem2;

                /* Go through every waypoint otherData node */
                iter3 = createIterator(tmpData2 -> waypoints);
                while ((elem3 = nextElement(&iter3)) != NULL) {
                    Waypoint* tmpData3 = (Waypoint*)elem3;

                    if(strcmp(tmpData3 -> name, "") != 0) {
                        counter++;
                    }

                    /* Go through every waypoints otherData node */
                    iter4 = createIterator(tmpData3 -> otherData);
                    while ((elem4 = nextElement(&iter4)) != NULL) {
                        counter++;
                    }
                }
            }

            /* Go through every route otherData node */
            iter2 = createIterator(tmpData1 -> otherData);
            while ((elem2 = nextElement(&iter2)) != NULL) {
                counter++;
            }
        }

        return(counter);
    }
    return(0);
}

Waypoint* getWaypoint(const GPXdoc* doc, char* name) {
    if (doc == NULL || name == NULL) {
        return(NULL);
    } else {
        void *elem;

        ListIterator iter = createIterator(doc -> waypoints);
        while ((elem = nextElement(&iter)) != NULL) {
            Waypoint* tmpData = (Waypoint*)elem;
            if (strcmp(tmpData -> name, name) == 0) {
                return(tmpData);
            }
        }

        return(NULL);
    }
}

Track* getTrack(const GPXdoc* doc, char* name) {
    if (doc == NULL || name == NULL) {
        return(NULL);
    } else {
        void *elem;

        ListIterator iter = createIterator(doc -> tracks);
        while ((elem = nextElement(&iter)) != NULL) {
            Track* tmpData = (Track*)elem;
            if (strcmp(tmpData -> name, name) == 0) {
                return(tmpData);
            }
        }

        return(NULL);
    }
}

Route* getRoute(const GPXdoc* doc, char* name) {
    if (doc == NULL || name == NULL) {
        return(NULL);
    } else {
        void *elem;

        ListIterator iter = createIterator(doc -> routes);
        while ((elem = nextElement(&iter)) != NULL) {
            Route* tmpData = (Route*)elem;
            if (strcmp(tmpData -> name, name) == 0) {
                return(tmpData);
            }
        }

        return(NULL);
    }
}

/***************************
* GPXData helper functions *
***************************/
void deleteGpxData(void* data) {
	GPXData* tmpGpxData;
	if (data == NULL) {
		return;
	}
	tmpGpxData = (GPXData*)data;
	free(tmpGpxData);
}

char* gpxDataToString(void* data) {
    char* tmpStr;
    GPXData* tmpGPXData;
    if (data == NULL) {
        return(NULL);
    }
    tmpGPXData = (GPXData*)data;
	tmpStr = (char*)malloc(sizeof(char)*8196);

    /* Printing GPXData */
	sprintf(tmpStr, "  GPXData: '%s' contents '%s'\n", tmpGPXData -> name, tmpGPXData -> value);
	return(tmpStr);
}

int compareGpxData(const void *first, const void *second) {
    return(0);
}

/****************************
* Waypoint helper functions *
****************************/
void deleteWaypoint(void* data) {
	Waypoint* tmpWaypoint;
	if (data == NULL) {
		return;
	}
	tmpWaypoint = (Waypoint*)data;
    freeList(tmpWaypoint -> otherData);
    if (tmpWaypoint -> name != NULL) {
        free(tmpWaypoint -> name);
    }
	free(tmpWaypoint);
}

char* waypointToString(void* data) {
    char* tmpStr;
    Waypoint* tmpWaypoint;
    if (data == NULL) {
        return(NULL);
    }
    tmpWaypoint = (Waypoint*)data;

	tmpStr = (char*)malloc(sizeof(char)*8196);
    /* Printing waypoint latitude and longitude */
	sprintf(tmpStr, "  Waypoint: '%s' lat=%f lon=%f\n", tmpWaypoint -> name, tmpWaypoint -> latitude, tmpWaypoint -> longitude);

    /* Printing otherData */
    void* elem;
    ListIterator iter = createIterator(tmpWaypoint -> otherData);
    while ((elem = nextElement(&iter)) != NULL){
        GPXData* tmpData = (GPXData*)elem;
        char* str = (tmpWaypoint -> otherData)->printData(tmpData);
        sprintf(tmpStr + strlen(tmpStr), "  %s", str);
        free(str);
    }
	return(tmpStr);
}

int compareWaypoints(const void *first, const void *second) {
    return(0);
}

/*************************
* Route helper functions *
**************************/
void deleteRoute(void* data) {
	Route* tmpRoute;
	if (data == NULL) {
		return;
	}
	tmpRoute = (Route*)data;
    freeList(tmpRoute -> waypoints);
    freeList(tmpRoute -> otherData);
    free(tmpRoute -> name);
	free(tmpRoute);
}

char* routeToString(void* data) {
    char* tmpStr;
    Route* tmpRoute;
    if (data == NULL) {
        return(NULL);
    }
    tmpRoute = (Route*)data;

	tmpStr = (char*)malloc(sizeof(char)*8196);
    /* Printing waypoint latitude and longitude */
	sprintf(tmpStr, "  Route: '%s'\n", tmpRoute -> name);

    /* Printing otherData */
    void* elem;
    ListIterator iter = createIterator(tmpRoute -> otherData);
    while ((elem = nextElement(&iter)) != NULL){
        GPXData* tmpData = (GPXData*)elem;
        char* str = (tmpRoute -> otherData)->printData(tmpData);
        sprintf(tmpStr + strlen(tmpStr), "  %s", str);
        free(str);
    }

    /* Printing waypoints */
    iter = createIterator(tmpRoute -> waypoints);
    while ((elem = nextElement(&iter)) != NULL){
        Waypoint* tmpData = (Waypoint*)elem;
        char* str = (tmpRoute -> waypoints)->printData(tmpData);
        sprintf(tmpStr + strlen(tmpStr), "  %s", str);
        free(str);
    }
	return(tmpStr);
}

int compareRoutes(const void *first, const void *second) {
    return(0);
}

/*********************************
* Track Segment helper functions *
*********************************/
void deleteTrackSegment(void* data) {
	TrackSegment* tmpTrackSegment;
	if (data == NULL) {
		return;
	}
	tmpTrackSegment = (TrackSegment*)data;
    freeList(tmpTrackSegment -> waypoints);
	free(tmpTrackSegment);
}

char* trackSegmentToString(void* data) {
    char* tmpStr;
    TrackSegment* tmpTrackSeg;
    if (data == NULL) {
        return(NULL);
    }
    tmpTrackSeg = (TrackSegment*)data;

	tmpStr = (char*)malloc(sizeof(char)*8196);
    sprintf(tmpStr, "  Trackseg: \n");

    /* Printing Waypoints */
    void* elem;
    ListIterator iter = createIterator(tmpTrackSeg -> waypoints);
    while ((elem = nextElement(&iter)) != NULL){
        Waypoint* tmpData = (Waypoint*)elem;
        char* str = (tmpTrackSeg -> waypoints)->printData(tmpData);
        sprintf(tmpStr + strlen(tmpStr), "  %s", str);
        free(str);
    }

	return(tmpStr);
}

int compareTrackSegments(const void *first, const void *second) {
    return(0);
}

/*************************
* Track helper functions *
**************************/
void deleteTrack(void* data) {
	Track* tmpTrack;
	if (data == NULL) {
		return;
	}
	tmpTrack = (Track*)data;
    freeList(tmpTrack -> otherData);
    freeList(tmpTrack -> segments);
    free(tmpTrack -> name);
	free(tmpTrack);
}

char* trackToString(void* data) {
    char* tmpStr;
    Track* tmpTrack;
    if (data == NULL) {
        return(NULL);
    }
    tmpTrack = (Track*)data;

	tmpStr = (char*)malloc(sizeof(char)*8196);
    /* Printing waypoint latitude and longitude */
	sprintf(tmpStr, "  Track: '%s'\n", tmpTrack -> name);

    /* Printing otherData */
    void* elem;
    ListIterator iter = createIterator(tmpTrack -> otherData);
    while ((elem = nextElement(&iter)) != NULL){
        GPXData* tmpData = (GPXData*)elem;
        char* str = (tmpTrack -> otherData)->printData(tmpData);
        sprintf(tmpStr + strlen(tmpStr), "  %s", str);
        free(str);
    }

    /* Printing segments */
    iter = createIterator(tmpTrack -> segments);
    while ((elem = nextElement(&iter)) != NULL){
        TrackSegment* tmpData = (TrackSegment*)elem;
        char* str = (tmpTrack -> segments)->printData(tmpData);
        sprintf(tmpStr + strlen(tmpStr), "%s", str);
        free(str);
    }
	return(tmpStr);
}

int compareTracks(const void *first, const void *second) {
    return(0);
}

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile) {
    /* Checking if the file names are NULL or an empty string, if either are, then return NULL */
    if (fileName == NULL || gpxSchemaFile == NULL) {
        return(NULL);
    } else if (strlen(fileName) == 0 || strlen(gpxSchemaFile) == 0) {
        return(NULL);
    }

    int validDoc = 0;
    xmlDoc *doc = NULL;
    xmlDoc *schemaDoc = NULL;
    xmlSchemaParserCtxt *schemaContext = NULL;
    xmlSchema *schema = NULL;
    xmlSchemaValidCtxt *validSchema = NULL;

    LIBXML_TEST_VERSION

    /* Parse the file .gpx file and get the DOM */
    doc = xmlReadFile(fileName, NULL, 0);
    if (doc == NULL) {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return(NULL);
    }

    /* Parse the .xsd schema file and get the DOM */
    schemaDoc = xmlReadFile(gpxSchemaFile, NULL, 0);
    if (schemaDoc == NULL) {
        xmlFreeDoc(doc);
        xmlFreeDoc(schemaDoc);
        xmlCleanupParser();
        return(NULL);
    }

    /* Create a parser context for the schema */
    schemaContext = xmlSchemaNewDocParserCtxt(schemaDoc);
    if (schemaContext == NULL) {
        xmlFreeDoc(doc);
        xmlFreeDoc(schemaDoc);
        xmlSchemaFreeParserCtxt(schemaContext);
        xmlCleanupParser();
        return(NULL);
    }

    /* Check if the schema itself is valid */
    schema = xmlSchemaParse(schemaContext);
    if (schema == NULL) {
        xmlFreeDoc(doc);
        xmlFreeDoc(schemaDoc);
        xmlSchemaFreeParserCtxt(schemaContext);
        xmlSchemaFree(schema);
        xmlCleanupParser();
        return(NULL);
    }

    /* Create a validation context for the schema */
    validSchema = xmlSchemaNewValidCtxt(schema);
    if (validSchema == NULL) {
        xmlFreeDoc(doc);
        xmlFreeDoc(schemaDoc);
        xmlSchemaFreeParserCtxt(schemaContext);
        xmlSchemaFree(schema);
        xmlSchemaFreeValidCtxt(validSchema);
        xmlCleanupParser();
        return(NULL);
    }

    /* Checking validity of the doc */
    validDoc = xmlSchemaValidateDoc(validSchema, doc);
    xmlFreeDoc(doc);
    xmlFreeDoc(schemaDoc);
    xmlSchemaFreeParserCtxt(schemaContext);
    xmlSchemaFree(schema);
    xmlSchemaFreeValidCtxt(validSchema);
    xmlCleanupParser();
    if (validDoc != 0) {
        return(NULL);
    }

    return(createGPXdoc(fileName));
}

bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile) {
    /* Checking if GPXdoc is NULL */
    if (doc == NULL) {
        return(NULL);
    }

    /* Checking for a valid file */
    if (gpxSchemaFile == NULL) {
        return(NULL);
    } else if (strlen(gpxSchemaFile) == 0) {
        return(NULL);
    }

    void *waypointElem;
    void *waypointOtherElem;
    void *routeElem;
    void *routeOtherElem;
    void *trackElem;
    void *trackOtherElem;
    void *segmentElem;
    ListIterator waypointIter;
    ListIterator waypointOtherIter;
    ListIterator routeIter;
    ListIterator routeOtherIter;
    ListIterator trackIter;
    ListIterator trackOtherIter;
    ListIterator segmentIter;
    Waypoint* tmpWaypointData = NULL;
    Route* tmpRouteData = NULL;
    Track* tmpTrackData = NULL;
    TrackSegment* tmpSegmentData = NULL;
    GPXData* tmpGpxData = NULL;
    char buf[1024];

    xmlDoc *validDoc = NULL;
    xmlNode *rootNode = NULL;
    xmlNode *node = NULL;
    xmlNode *childNode = NULL;
    xmlNode *childChildNode = NULL;
    xmlNs *nameSpace = NULL;

    LIBXML_TEST_VERSION;

    /*********************
    * CREATING ROOT NODE *
    *********************/
    validDoc = xmlNewDoc(BAD_CAST "1.0");
    rootNode = xmlNewNode(NULL, BAD_CAST "gpx");
    xmlDocSetRootElement(validDoc, rootNode);

    /* Parsing namespace */
    if (doc->namespace == NULL) {
        xmlFreeDoc(validDoc);
        xmlCleanupParser();
        return(NULL);
    } else if (strlen(doc->namespace) == 0) {
        xmlFreeDoc(validDoc);
        xmlCleanupParser();
        return(NULL);       
    }
    nameSpace = xmlNewNs(rootNode, BAD_CAST doc->namespace, NULL);
    xmlSetNs(rootNode, nameSpace);

    /* Parsing version */
    sprintf(buf, "%0.1f", doc->version);
    xmlNewProp(rootNode, BAD_CAST "version", BAD_CAST buf);

    /* Parsing creator */
    if (doc->creator == NULL) {
        xmlFreeDoc(validDoc);
        xmlCleanupParser();
        return(NULL);
    } else if (strlen(doc->creator) == 0) {
        xmlFreeDoc(validDoc);
        xmlCleanupParser();
        return(NULL);
    }
    xmlNewProp(rootNode, BAD_CAST "creator", BAD_CAST doc->creator);

    /*********************
    * CREATING WAYPOINTS *
    *********************/
    /* Checking if the waypoints list is NULL */
    if (doc->waypoints == NULL) {
        xmlFreeDoc(validDoc);
        xmlCleanupParser();
        return(NULL);
    }

    /* Parsing the waypoints */
    waypointIter = createIterator(doc->waypoints);
    while ((waypointElem = nextElement(&waypointIter)) != NULL) {
        node = xmlNewChild(rootNode, rootNode->ns, BAD_CAST "wpt", NULL);
        tmpWaypointData = (Waypoint*)waypointElem;
        
        /* Checking if otherData list is NULL */
        if (tmpWaypointData->otherData == NULL) {
            xmlFreeDoc(validDoc);
            xmlCleanupParser();
            return(NULL);
        }

        /* Parsing wpt otherData that goes before the name */
        waypointOtherIter = createIterator(tmpWaypointData->otherData);
        while ((waypointOtherElem = nextElement(&waypointOtherIter)) != NULL) {
            tmpGpxData = (GPXData*)waypointOtherElem;

            /* Parsing a GPXData name */
            if (tmpGpxData->name == NULL) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            } else if (strlen(tmpGpxData->name) == 0) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            }

            /* Parsing a GPXData value */
            if (tmpGpxData->value == NULL) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            } else if (strlen(tmpGpxData->value) == 0) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            }

            if (strcmp(tmpGpxData->name, "ele") == 0 || strcmp(tmpGpxData->name, "time") == 0 || strcmp(tmpGpxData->name, "magvar") == 0 || strcmp(tmpGpxData->name, "geoidheight") == 0) {
                xmlNewChild(node, rootNode->ns, BAD_CAST tmpGpxData->name, BAD_CAST tmpGpxData->value);
            }
        }
        
        /* Parsing wpt name */
        if (tmpWaypointData->name == NULL) {
            xmlFreeDoc(validDoc);
            xmlCleanupParser();
            return(NULL);       
        }
        if (strlen(tmpWaypointData->name) != 0) {
            xmlNewChild(node, rootNode->ns, BAD_CAST "name", BAD_CAST tmpWaypointData->name);
        }

        /* Parsing wpt latitude */
        sprintf(buf, "%0.5f", tmpWaypointData->latitude);
        xmlNewProp(node, BAD_CAST "lat", BAD_CAST buf);

        /* Parsing wpt longitude */
        sprintf(buf, "%0.5f", tmpWaypointData->longitude);
        xmlNewProp(node, BAD_CAST "lon", BAD_CAST buf);

        /* Parsing wpt otherData that goes after the name */
        waypointOtherIter = createIterator(tmpWaypointData->otherData);
        while ((waypointOtherElem = nextElement(&waypointOtherIter)) != NULL) {
            tmpGpxData = (GPXData*)waypointOtherElem;

            /* Parsing a GPXData name */
            if (tmpGpxData->name == NULL) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            } else if (strlen(tmpGpxData->name) == 0) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            }

            /* Parsing a GPXData value */
            if (tmpGpxData->value == NULL) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            } else if (strlen(tmpGpxData->value) == 0) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            }

            if (strcmp(tmpGpxData->name, "ele") != 0 && strcmp(tmpGpxData->name, "time") != 0 && strcmp(tmpGpxData->name, "magvar") != 0 && strcmp(tmpGpxData->name, "geoidheight") != 0) {
                xmlNewChild(node, rootNode->ns, BAD_CAST tmpGpxData->name, BAD_CAST tmpGpxData->value);
            }
        }
    }

    /******************
    * CREATING ROUTES *
    ******************/
    /* Checking if the routes list is NULL */
    if (doc->routes == NULL) {
        xmlFreeDoc(validDoc);
        xmlCleanupParser();
        return(NULL);
    }

    /* Parsing the routes */
    routeIter = createIterator(doc->routes);
    while ((routeElem = nextElement(&routeIter)) != NULL) {
        node = xmlNewChild(rootNode, rootNode->ns, BAD_CAST "rte", NULL);
        tmpRouteData = (Route*)routeElem;

        /* Parsing route name */
        if (tmpRouteData->name == NULL) {
            xmlFreeDoc(validDoc);
            xmlCleanupParser();
            return(NULL);       
        }
        if (strlen(tmpRouteData->name) != 0) {
            xmlNewChild(node, rootNode->ns, BAD_CAST "name", BAD_CAST tmpRouteData->name);
        }

        /* Parsing route otherData */
        /* Checking if otherData list is NULL */
        if (tmpRouteData->otherData == NULL) {
            xmlFreeDoc(validDoc);
            xmlCleanupParser();
            return(NULL);
        }

        routeOtherIter = createIterator(tmpRouteData->otherData);
        while ((routeOtherElem = nextElement(&routeOtherIter)) != NULL) {
            GPXData* tmpGPXData = (GPXData*)routeOtherElem;

            /* Parsing a GPXData name */
            if (tmpGPXData->name == NULL) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            } else if (strlen(tmpGPXData->name) == 0) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            }

            /* Parsing a GPXData value */
            if (tmpGPXData->value == NULL) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            } else if (strlen(tmpGPXData->value) == 0) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            }
            xmlNewChild(node, rootNode->ns, BAD_CAST tmpGPXData->name, BAD_CAST tmpGPXData->value);
        }

        /* Parsing route waypoints */
        /* Checking if the waypoints list is NULL */
        if (tmpRouteData->waypoints == NULL) {
            xmlFreeDoc(validDoc);
            xmlCleanupParser();
            return(NULL);
        }

        /* Parsing the waypoints */
        waypointIter = createIterator(tmpRouteData->waypoints);
        while ((waypointElem = nextElement(&waypointIter)) != NULL) {
            childNode = xmlNewChild(node, rootNode->ns, BAD_CAST "rtept", NULL);
            tmpWaypointData = (Waypoint*)waypointElem;

            /* Checking if otherData list is NULL */
            if (tmpWaypointData->otherData == NULL) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            }

            /* Parsing wpt otherData that goes before the name */
            waypointOtherIter = createIterator(tmpWaypointData->otherData);
            while ((waypointOtherElem = nextElement(&waypointOtherIter)) != NULL) {
                tmpGpxData = (GPXData*)waypointOtherElem;

                /* Parsing a GPXData name */
                if (tmpGpxData->name == NULL) {
                    xmlFreeDoc(validDoc);
                    xmlCleanupParser();
                    return(NULL);
                } else if (strlen(tmpGpxData->name) == 0) {
                    xmlFreeDoc(validDoc);
                    xmlCleanupParser();
                    return(NULL);
                }

                /* Parsing a GPXData value */
                if (tmpGpxData->value == NULL) {
                    xmlFreeDoc(validDoc);
                    xmlCleanupParser();
                    return(NULL);
                } else if (strlen(tmpGpxData->value) == 0) {
                    xmlFreeDoc(validDoc);
                    xmlCleanupParser();
                    return(NULL);
                }

                if (strcmp(tmpGpxData->name, "ele") == 0 || strcmp(tmpGpxData->name, "time") == 0 || strcmp(tmpGpxData->name, "magvar") == 0 || strcmp(tmpGpxData->name, "geoidheight") == 0) {
                    xmlNewChild(childNode, rootNode->ns, BAD_CAST tmpGpxData->name, BAD_CAST tmpGpxData->value);
                }
            }

            /* Parsing wpt name */
            if (tmpWaypointData->name == NULL) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);       
            }
            if (strlen(tmpWaypointData->name) != 0) {
                xmlNewChild(childNode, rootNode->ns, BAD_CAST "name", BAD_CAST tmpWaypointData->name);
            }

            /* Parsing wpt latitude */
            sprintf(buf, "%0.5f", tmpWaypointData->latitude);
            xmlNewProp(childNode, BAD_CAST "lat", BAD_CAST buf);

            /* Parsing wpt longitude */
            sprintf(buf, "%0.5f", tmpWaypointData->longitude);
            xmlNewProp(childNode, BAD_CAST "lon", BAD_CAST buf);

            /* Parsing wpt otherData that goes after the name */
            waypointOtherIter = createIterator(tmpWaypointData->otherData);
            while ((waypointOtherElem = nextElement(&waypointOtherIter)) != NULL) {
                tmpGpxData = (GPXData*)waypointOtherElem;

                /* Parsing a GPXData name */
                if (tmpGpxData->name == NULL) {
                    xmlFreeDoc(validDoc);
                    xmlCleanupParser();
                    return(NULL);
                } else if (strlen(tmpGpxData->name) == 0) {
                    xmlFreeDoc(validDoc);
                    xmlCleanupParser();
                    return(NULL);
                }

                /* Parsing a GPXData value */
                if (tmpGpxData->value == NULL) {
                    xmlFreeDoc(validDoc);
                    xmlCleanupParser();
                    return(NULL);
                } else if (strlen(tmpGpxData->value) == 0) {
                    xmlFreeDoc(validDoc);
                    xmlCleanupParser();
                    return(NULL);
                }

                if (strcmp(tmpGpxData->name, "ele") != 0 && strcmp(tmpGpxData->name, "time") != 0 && strcmp(tmpGpxData->name, "magvar") != 0 && strcmp(tmpGpxData->name, "geoidheight") != 0) {
                    xmlNewChild(childNode, rootNode->ns, BAD_CAST tmpGpxData->name, BAD_CAST tmpGpxData->value);
                }
            }
        }
    }

    /******************
    * CREATING TRACKS *
    ******************/
    /* Checking if the tracks list is NULL */
    if (doc->tracks == NULL) {
        xmlFreeDoc(validDoc);
        xmlCleanupParser();
        return(NULL);
    }

    /* Parsing the tracks */
    trackIter = createIterator(doc->tracks);
    while ((trackElem = nextElement(&trackIter)) != NULL) {
        node = xmlNewChild(rootNode, rootNode->ns, BAD_CAST "trk", NULL);
        tmpTrackData = (Track*)trackElem;

        /* Parsing route name */
        if (tmpTrackData->name == NULL) {
            xmlFreeDoc(validDoc);
            xmlCleanupParser();
            return(NULL);       
        }
        if (strlen(tmpTrackData->name) != 0) {
            xmlNewChild(node, rootNode->ns, BAD_CAST "name", BAD_CAST tmpTrackData->name);
        }

        /* Parsing track otherData */
        /* Checking if otherData list is NULL */
        if (tmpTrackData->otherData == NULL) {
            xmlFreeDoc(validDoc);
            xmlCleanupParser();
            return(NULL);
        }

        trackOtherIter = createIterator(tmpTrackData->otherData);
        while ((trackOtherElem = nextElement(&trackOtherIter)) != NULL) {
            GPXData* tmpGPXData = (GPXData*)trackOtherElem;

            /* Parsing a GPXData name */
            if (tmpGPXData->name == NULL) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            } else if (strlen(tmpGPXData->name) == 0) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            }

            /* Parsing a GPXData value */
            if (tmpGPXData->value == NULL) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            } else if (strlen(tmpGPXData->value) == 0) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            }
            xmlNewChild(node, rootNode->ns, BAD_CAST tmpGPXData->name, BAD_CAST tmpGPXData->value);
        }

        /* Parsing track segments */
        /* Checking if the waypoints list is NULL */
        if (tmpTrackData->segments == NULL) {
            xmlFreeDoc(validDoc);
            xmlCleanupParser();
            return(NULL);
        }

        /* Parsing the waypoints */
        segmentIter = createIterator(tmpTrackData->segments);
        while ((segmentElem = nextElement(&segmentIter)) != NULL) {
            childNode = xmlNewChild(node, rootNode->ns, BAD_CAST "trkseg", NULL);
            tmpSegmentData = (TrackSegment*)segmentElem;

            /* Parsing segment waypoints */
            /* Checking if the segments list is NULL */
            if (tmpSegmentData->waypoints == NULL) {
                xmlFreeDoc(validDoc);
                xmlCleanupParser();
                return(NULL);
            }

            /* Parsing the waypoints */
            waypointIter = createIterator(tmpSegmentData->waypoints);
            while ((waypointElem = nextElement(&waypointIter)) != NULL) {
                childChildNode = xmlNewChild(childNode, rootNode->ns, BAD_CAST "trkpt", NULL);
                tmpWaypointData = (Waypoint*)waypointElem;

                /* Checking if otherData list is NULL */
                if (tmpWaypointData->otherData == NULL) {
                    xmlFreeDoc(validDoc);
                    xmlCleanupParser();
                    return(NULL);
                }

                /* Parsing wpt otherData that goes before the name */
                waypointOtherIter = createIterator(tmpWaypointData->otherData);
                while ((waypointOtherElem = nextElement(&waypointOtherIter)) != NULL) {
                    tmpGpxData = (GPXData*)waypointOtherElem;

                    /* Parsing a GPXData name */
                    if (tmpGpxData->name == NULL) {
                        xmlFreeDoc(validDoc);
                        xmlCleanupParser();
                        return(NULL);
                    } else if (strlen(tmpGpxData->name) == 0) {
                        xmlFreeDoc(validDoc);
                        xmlCleanupParser();
                        return(NULL);
                    }

                    /* Parsing a GPXData value */
                    if (tmpGpxData->value == NULL) {
                        xmlFreeDoc(validDoc);
                        xmlCleanupParser();
                        return(NULL);
                    } else if (strlen(tmpGpxData->value) == 0) {
                        xmlFreeDoc(validDoc);
                        xmlCleanupParser();
                        return(NULL);
                    }

                    if (strcmp(tmpGpxData->name, "ele") == 0 || strcmp(tmpGpxData->name, "time") == 0 || strcmp(tmpGpxData->name, "magvar") == 0 || strcmp(tmpGpxData->name, "geoidheight") == 0) {
                        xmlNewChild(childChildNode, rootNode->ns, BAD_CAST tmpGpxData->name, BAD_CAST tmpGpxData->value);
                    }
                }

                /* Parsing wpt name */
                if (tmpWaypointData->name == NULL) {
                    xmlFreeDoc(validDoc);
                    xmlCleanupParser();
                    return(NULL);       
                }
                if (strlen(tmpWaypointData->name) != 0) {
                    xmlNewChild(childChildNode, rootNode->ns, BAD_CAST "name", BAD_CAST tmpWaypointData->name);
                }

                /* Parsing wpt latitude */
                sprintf(buf, "%0.5f", tmpWaypointData->latitude);
                xmlNewProp(childChildNode, BAD_CAST "lat", BAD_CAST buf);

                /* Parsing wpt longitude */
                sprintf(buf, "%0.5f", tmpWaypointData->longitude);
                xmlNewProp(childChildNode, BAD_CAST "lon", BAD_CAST buf);

                /* Parsing wpt otherData that goes after the name */
                waypointOtherIter = createIterator(tmpWaypointData->otherData);
                while ((waypointOtherElem = nextElement(&waypointOtherIter)) != NULL) {
                    tmpGpxData = (GPXData*)waypointOtherElem;

                    /* Parsing a GPXData name */
                    if (tmpGpxData->name == NULL) {
                        xmlFreeDoc(validDoc);
                        xmlCleanupParser();
                        return(NULL);
                    } else if (strlen(tmpGpxData->name) == 0) {
                        xmlFreeDoc(validDoc);
                        xmlCleanupParser();
                        return(NULL);
                    }

                    /* Parsing a GPXData value */
                    if (tmpGpxData->value == NULL) {
                        xmlFreeDoc(validDoc);
                        xmlCleanupParser();
                        return(NULL);
                    } else if (strlen(tmpGpxData->value) == 0) {
                        xmlFreeDoc(validDoc);
                        xmlCleanupParser();
                        return(NULL);
                    }

                    if (strcmp(tmpGpxData->name, "ele") != 0 && strcmp(tmpGpxData->name, "time") != 0 && strcmp(tmpGpxData->name, "magvar") != 0 && strcmp(tmpGpxData->name, "geoidheight") != 0) {
                        xmlNewChild(childChildNode, rootNode->ns, BAD_CAST tmpGpxData->name, BAD_CAST tmpGpxData->value);
                    }
                }
            }
        }
    }

    int isValid = 0;
    xmlDoc *schemaDoc = NULL;
    xmlSchemaParserCtxt *schemaContext = NULL;
    xmlSchema *schema = NULL;
    xmlSchemaValidCtxt *validSchema = NULL;

    LIBXML_TEST_VERSION

    /* Parse the .xsd schema file and get the DOM */
    schemaDoc = xmlReadFile(gpxSchemaFile, NULL, 0);
    if (schemaDoc == NULL) {
        xmlFreeDoc(validDoc);
        xmlFreeDoc(schemaDoc);
        xmlCleanupParser();
        return(NULL);
    }

    /* Create a parser context for the schema */
    schemaContext = xmlSchemaNewDocParserCtxt(schemaDoc);
    if (schemaContext == NULL) {
        xmlFreeDoc(validDoc);
        xmlFreeDoc(schemaDoc);
        xmlSchemaFreeParserCtxt(schemaContext);
        xmlCleanupParser();
        return(NULL);
    }

    /* Check if the schema itself is valid */
    schema = xmlSchemaParse(schemaContext);
    if (schema == NULL) {
        xmlFreeDoc(validDoc);
        xmlFreeDoc(schemaDoc);
        xmlSchemaFreeParserCtxt(schemaContext);
        xmlSchemaFree(schema);
        xmlCleanupParser();
        return(NULL);
    }

    /* Create a validation context for the schema */
    validSchema = xmlSchemaNewValidCtxt(schema);
    if (validSchema == NULL) {
        xmlFreeDoc(validDoc);
        xmlFreeDoc(schemaDoc);
        xmlSchemaFreeParserCtxt(schemaContext);
        xmlSchemaFree(schema);
        xmlSchemaFreeValidCtxt(validSchema);
        xmlCleanupParser();
        return(NULL);
    }

    /* Checking validity of the doc */
    isValid = xmlSchemaValidateDoc(validSchema, validDoc);
    xmlFreeDoc(validDoc);
    xmlFreeDoc(schemaDoc);
    xmlSchemaFreeParserCtxt(schemaContext);
    xmlSchemaFree(schema);
    xmlSchemaFreeValidCtxt(validSchema);
    xmlCleanupParser();
    if (isValid != 0) {
        return(false);
    } else {
        return(true);
    }
}

bool writeGPXdoc(GPXdoc* doc, char* fileName) {
    /* Checking if GPXdoc is NULL */
    if (doc == NULL) {
        return(false);
    }

    /* Checking for a valid file */
    if (fileName == NULL) {
        return(false);
    } else if (strlen(fileName) == 0) {
        return(false);
    }

    void *waypointElem;
    void *waypointOtherElem;
    void *routeElem;
    void *routeOtherElem;
    void *trackElem;
    void *trackOtherElem;
    void *segmentElem;
    ListIterator waypointIter;
    ListIterator waypointOtherIter;
    ListIterator routeIter;
    ListIterator routeOtherIter;
    ListIterator trackIter;
    ListIterator trackOtherIter;
    ListIterator segmentIter;
    Waypoint* tmpWaypointData = NULL;
    Route* tmpRouteData = NULL;
    Track* tmpTrackData = NULL;
    TrackSegment* tmpSegmentData = NULL;
    GPXData* tmpGpxData = NULL;
    char buf[1024];

    xmlDoc *validDoc = NULL;
    xmlNode *rootNode = NULL;
    xmlNode *node = NULL;
    xmlNode *childNode = NULL;
    xmlNode *childChildNode = NULL;
    xmlNs *nameSpace = NULL;

    LIBXML_TEST_VERSION;

    /*********************
    * CREATING ROOT NODE *
    *********************/
    validDoc = xmlNewDoc(BAD_CAST "1.0");
    rootNode = xmlNewNode(NULL, BAD_CAST "gpx");
    xmlDocSetRootElement(validDoc, rootNode);

    /* Parsing namespace */
    nameSpace = xmlNewNs(rootNode, BAD_CAST doc->namespace, NULL);
    xmlSetNs(rootNode, nameSpace);

    /* Parsing version */
    sprintf(buf, "%0.1f", doc->version);
    xmlNewProp(rootNode, BAD_CAST "version", BAD_CAST buf);

    /* Parsing creator */
    xmlNewProp(rootNode, BAD_CAST "creator", BAD_CAST doc->creator);

    /*********************
    * CREATING WAYPOINTS *
    *********************/
    /* Parsing the waypoints */
    waypointIter = createIterator(doc->waypoints);
    while ((waypointElem = nextElement(&waypointIter)) != NULL) {
        node = xmlNewChild(rootNode, rootNode->ns, BAD_CAST "wpt", NULL);
        tmpWaypointData = (Waypoint*)waypointElem;

        /* Parsing wpt otherData that goes before the name */
        waypointOtherIter = createIterator(tmpWaypointData->otherData);
        while ((waypointOtherElem = nextElement(&waypointOtherIter)) != NULL) {
            tmpGpxData = (GPXData*)waypointOtherElem;
            if (strcmp(tmpGpxData->name, "ele") == 0 || strcmp(tmpGpxData->name, "time") == 0 || strcmp(tmpGpxData->name, "magvar") == 0 || strcmp(tmpGpxData->name, "geoidheight") == 0) {
                xmlNewChild(node, rootNode->ns, BAD_CAST tmpGpxData->name, BAD_CAST tmpGpxData->value);
            }
        }

        /* Parsing wpt name */
        if (tmpWaypointData->name != NULL) {
            if (strlen(tmpWaypointData->name) != 0) {
                xmlNewChild(node, rootNode->ns, BAD_CAST "name", BAD_CAST tmpWaypointData->name);
            }
        }

        /* Parsing wpt latitude */
        sprintf(buf, "%0.5f", tmpWaypointData->latitude);
        xmlNewProp(node, BAD_CAST "lat", BAD_CAST buf);

        /* Parsing wpt longitude */
        sprintf(buf, "%0.5f", tmpWaypointData->longitude);
        xmlNewProp(node, BAD_CAST "lon", BAD_CAST buf);

        /* Parsing wpt otherData */
        /* Checking if otherData list is NULL */
        waypointOtherIter = createIterator(tmpWaypointData->otherData);
        while ((waypointOtherElem = nextElement(&waypointOtherIter)) != NULL) {
            tmpGpxData = (GPXData*)waypointOtherElem;
            if (strcmp(tmpGpxData->name, "ele") != 0 && strcmp(tmpGpxData->name, "time") != 0 && strcmp(tmpGpxData->name, "magvar") != 0 && strcmp(tmpGpxData->name, "geoidheight") != 0) {
                xmlNewChild(node, rootNode->ns, BAD_CAST tmpGpxData->name, BAD_CAST tmpGpxData->value);
            }
        }
    }

    /******************
    * CREATING ROUTES *
    ******************/
    /* Checking if the routes list is NULL */
    /* Parsing the routes */
    routeIter = createIterator(doc->routes);
    while ((routeElem = nextElement(&routeIter)) != NULL) {
        node = xmlNewChild(rootNode, rootNode->ns, BAD_CAST "rte", NULL);
        tmpRouteData = (Route*)routeElem;

        /* Parsing route name */
        if (tmpRouteData->name != NULL) {
            if (strlen(tmpRouteData->name) != 0) {
                xmlNewChild(node, rootNode->ns, BAD_CAST "name", BAD_CAST tmpRouteData->name);
            }
        }

        /* Parsing route otherData */
        /* Checking if otherData list is NULL */
        routeOtherIter = createIterator(tmpRouteData->otherData);
        while ((routeOtherElem = nextElement(&routeOtherIter)) != NULL) {
            GPXData* tmpGPXData = (GPXData*)routeOtherElem;
            xmlNewChild(node, rootNode->ns, BAD_CAST tmpGPXData->name, BAD_CAST tmpGPXData->value);
        }

        /* Parsing route waypoints */
        /* Parsing the waypoints */
        waypointIter = createIterator(tmpRouteData->waypoints);
        while ((waypointElem = nextElement(&waypointIter)) != NULL) {
            childNode = xmlNewChild(node, rootNode->ns, BAD_CAST "rtept", NULL);
            tmpWaypointData = (Waypoint*)waypointElem;

            /* Parsing wpt otherData that goes before the name */
            waypointOtherIter = createIterator(tmpWaypointData->otherData);
            while ((waypointOtherElem = nextElement(&waypointOtherIter)) != NULL) {
                tmpGpxData = (GPXData*)waypointOtherElem;
                if (strcmp(tmpGpxData->name, "ele") == 0 || strcmp(tmpGpxData->name, "time") == 0 || strcmp(tmpGpxData->name, "magvar") == 0 || strcmp(tmpGpxData->name, "geoidheight") == 0) {
                    xmlNewChild(childNode, rootNode->ns, BAD_CAST tmpGpxData->name, BAD_CAST tmpGpxData->value);
                }
            }

            /* Parsing wpt name */
            if (tmpWaypointData->name != NULL) {
                if (strlen(tmpWaypointData->name) != 0) {
                    xmlNewChild(childNode, rootNode->ns, BAD_CAST "name", BAD_CAST tmpWaypointData->name);
                }
            }

            /* Parsing wpt latitude */
            sprintf(buf, "%0.5f", tmpWaypointData->latitude);
            xmlNewProp(childNode, BAD_CAST "lat", BAD_CAST buf);

            /* Parsing wpt longitude */
            sprintf(buf, "%0.5f", tmpWaypointData->longitude);
            xmlNewProp(childNode, BAD_CAST "lon", BAD_CAST buf);

            /* Parsing wpt otherData that goes after the name */
            waypointOtherIter = createIterator(tmpWaypointData->otherData);
            while ((waypointOtherElem = nextElement(&waypointOtherIter)) != NULL) {
                tmpGpxData = (GPXData*)waypointOtherElem;
                if (strcmp(tmpGpxData->name, "ele") != 0 && strcmp(tmpGpxData->name, "time") != 0 && strcmp(tmpGpxData->name, "magvar") != 0 && strcmp(tmpGpxData->name, "geoidheight") != 0) {
                    xmlNewChild(childNode, rootNode->ns, BAD_CAST tmpGpxData->name, BAD_CAST tmpGpxData->value);
                }
            }
        }
    }

    /******************
    * CREATING TRACKS *
    ******************/
    /* Parsing the tracks */
    trackIter = createIterator(doc->tracks);
    while ((trackElem = nextElement(&trackIter)) != NULL) {
        node = xmlNewChild(rootNode, rootNode->ns, BAD_CAST "trk", NULL);
        tmpTrackData = (Track*)trackElem;

        /* Parsing route name */
        if (tmpTrackData->name != NULL) {
            if (strlen(tmpTrackData->name) != 0) {
                xmlNewChild(node, rootNode->ns, BAD_CAST "name", BAD_CAST tmpTrackData->name);
            }
        }

        /* Parsing track otherData */
        trackOtherIter = createIterator(tmpTrackData->otherData);
        while ((trackOtherElem = nextElement(&trackOtherIter)) != NULL) {
            GPXData* tmpGPXData = (GPXData*)trackOtherElem;
            xmlNewChild(node, rootNode->ns, BAD_CAST tmpGPXData->name, BAD_CAST tmpGPXData->value);
        }

        /* Parsing track segments */
        /* Parsing the waypoints */
        segmentIter = createIterator(tmpTrackData->segments);
        while ((segmentElem = nextElement(&segmentIter)) != NULL) {
            childNode = xmlNewChild(node, rootNode->ns, BAD_CAST "trkseg", NULL);
            tmpSegmentData = (TrackSegment*)segmentElem;

            /* Parsing segment waypoints */
            /* Parsing the waypoints */
            waypointIter = createIterator(tmpSegmentData->waypoints);
            while ((waypointElem = nextElement(&waypointIter)) != NULL) {
                childChildNode = xmlNewChild(childNode, rootNode->ns, BAD_CAST "trkpt", NULL);
                tmpWaypointData = (Waypoint*)waypointElem;

                /* Parsing wpt otherData that goes before the name */
                waypointOtherIter = createIterator(tmpWaypointData->otherData);
                while ((waypointOtherElem = nextElement(&waypointOtherIter)) != NULL) {
                    tmpGpxData = (GPXData*)waypointOtherElem;
                    if (strcmp(tmpGpxData->name, "ele") == 0 || strcmp(tmpGpxData->name, "time") == 0 || strcmp(tmpGpxData->name, "magvar") == 0 || strcmp(tmpGpxData->name, "geoidheight") == 0) {
                        xmlNewChild(childChildNode, rootNode->ns, BAD_CAST tmpGpxData->name, BAD_CAST tmpGpxData->value);
                    }
                }

                /* Parsing wpt name */
                if (tmpWaypointData->name != NULL) {
                    if (strlen(tmpWaypointData->name) != 0) {
                        xmlNewChild(childChildNode, rootNode->ns, BAD_CAST "name", BAD_CAST tmpWaypointData->name);
                    }
                }

                /* Parsing wpt latitude */
                sprintf(buf, "%0.5f", tmpWaypointData->latitude);
                xmlNewProp(childChildNode, BAD_CAST "lat", BAD_CAST buf);

                /* Parsing wpt longitude */
                sprintf(buf, "%0.5f", tmpWaypointData->longitude);
                xmlNewProp(childChildNode, BAD_CAST "lon", BAD_CAST buf);

                /* Parsing wpt otherData that goes after the name */
                waypointOtherIter = createIterator(tmpWaypointData->otherData);
                while ((waypointOtherElem = nextElement(&waypointOtherIter)) != NULL) {
                    tmpGpxData = (GPXData*)waypointOtherElem;
                    if (strcmp(tmpGpxData->name, "ele") != 0 && strcmp(tmpGpxData->name, "time") != 0 && strcmp(tmpGpxData->name, "magvar") != 0 && strcmp(tmpGpxData->name, "geoidheight") != 0) {
                        xmlNewChild(childChildNode, rootNode->ns, BAD_CAST tmpGpxData->name, BAD_CAST tmpGpxData->value);
                    }
                }
            }
        }
    }

    xmlSaveFormatFileEnc(fileName, validDoc, "UTF-8", 1);
    xmlFreeDoc(validDoc);
    xmlCleanupParser();
    return (true);
}

float round10(float len) {
  int val = (len+5)/10;
  val *= 10;
  return (val);
}

float getRouteLen(const Route* rt) {
    if (rt == NULL) {
        return (0);
    }
    int i = 0;
    int numPts = getLength(rt->waypoints);
    double result = 0.0;
    double coordsArray[2][numPts];

    /* Iterating through the waypoints and storing them in an array */
    void *elem;
    ListIterator iter = createIterator(rt->waypoints);
    while ((elem = nextElement(&iter)) != NULL) {
        Waypoint* tmpWaypointData = (Waypoint *)elem;
        coordsArray[0][i] = tmpWaypointData->longitude;
        coordsArray[1][i] = tmpWaypointData->latitude;
        i++;
    }

    /* Calculating distance between coordinates */
    for (int i = 0; i < numPts - 1; i++) {
        result += getDistVal(coordsArray[1][i], coordsArray[0][i], coordsArray[1][i+1], coordsArray[0][i+1]);
    }
    return (result*1000);

}

float getTrackLen(const Track* tr) {
    if (tr == NULL) {
        return (0);
    } else if (tr->segments == NULL) {
        return (0);
    }

    int i = 0;
    int firstRun = 0;
    double result = 0;

    double lastPtLon = -1;
    double lastPtLat = -1;
    double firstPtLon = -1;
    double firstPtLat = -1;

    /* Iterating through the track and using the segments */
    void *elemTr;
    ListIterator iterTr = createIterator(tr->segments);
    while ((elemTr = nextElement(&iterTr)) != NULL) {
        TrackSegment* tmpTrackSegmentData = (TrackSegment *)elemTr;
        int numPts = 0;
        if (tmpTrackSegmentData == NULL || tmpTrackSegmentData->waypoints == NULL) {
            numPts = 0;
        } else {
            numPts = getLength(tmpTrackSegmentData->waypoints);
        }

        double coordsArray[2][numPts];

        /* Iterating through the waypoints and storing them in an array */
        void *elemWp;
        i = 0;
        ListIterator iterWp = createIterator(tmpTrackSegmentData->waypoints);
        while ((elemWp = nextElement(&iterWp)) != NULL) {
            Waypoint* tmpWaypointData = (Waypoint *)elemWp;
            coordsArray[0][i] = tmpWaypointData->longitude;
            coordsArray[1][i] = tmpWaypointData->latitude;
            i++;
        }
        firstPtLon = coordsArray[0][0];
        firstPtLat = coordsArray[1][0];

        /* Calculating adding distance between segments */
        if (firstRun == 1) {
            result += getDistVal(firstPtLat, firstPtLon, lastPtLat, lastPtLon);
        }

        /* Calculating distance between coordinates */
        for (int i = 0; i < numPts - 1; i++) {
            result += getDistVal(coordsArray[1][i], coordsArray[0][i], coordsArray[1][i+1], coordsArray[0][i+1]);
        }

        lastPtLon = coordsArray[0][numPts-1];
        lastPtLat = coordsArray[1][numPts-1];
        firstRun = 1;
    }
    return (result*1000);
}

int numRoutesWithLength(const GPXdoc* doc, float len, float delta) {
    if (doc == NULL || len < 0 || delta < 0) {
        return (0);
    }
    int result = 0;

    /* Iterating through the routes */
    void *elemRt;
    ListIterator iterRt = createIterator(doc->routes);
    while ((elemRt = nextElement(&iterRt)) != NULL) {
        Route* tmpRouteData = (Route *)elemRt;  
        double distance = getRouteLen(tmpRouteData);

        /* Checking if the route distance is within tolerance */
        if (fabs(distance-len) <= delta) {
            result++;
        }
    }
    return (result);
}

int numTracksWithLength(const GPXdoc* doc, float len, float delta) {
    if (doc == NULL || len < 0 || delta < 0) {
        return (0);
    }
    int result = 0;

    /* Iterating through the tracks */
    void *elemTr;
    ListIterator iterTr = createIterator(doc->tracks);
    while ((elemTr = nextElement(&iterTr)) != NULL) {
        Track* tmpTrackData = (Track *)elemTr;  
        double distance = getTrackLen(tmpTrackData);

        /* Checking if the track distance is within tolerance */
        if (fabs(distance-len) <= delta) {
            result++;
        }
    }
    return (result);
}

bool isLoopRoute(const Route* rt, float delta) {
    if (rt == NULL || delta < 0) {
        return (false);
    } else if (getLength(rt->waypoints) < 4) {
        return (false);
    }

    int i = 0;
    int numPts = getLength(rt->waypoints);
    double coordsArray[2][numPts];

    /* Iterating through the waypoints and storing them in an array */
    void *elem;
    ListIterator iter = createIterator(rt->waypoints);
    while ((elem = nextElement(&iter)) != NULL) {
        Waypoint* tmpWaypointData = (Waypoint *)elem;
        coordsArray[0][i] = tmpWaypointData->longitude;
        coordsArray[1][i] = tmpWaypointData->latitude;
        i++;
    }

    /* Calculating distance between first and last point */
    double distance = getDistVal(coordsArray[1][0], coordsArray[0][0], coordsArray[1][numPts-1], coordsArray[0][numPts-1]);
    if (fabs(distance*1000) <= delta) {
        return (true);
    } else {
        return (false);
    }
}

bool isLoopTrack(const Track* tr, float delta) {
    if (tr == NULL) {
        return (0);
    }

    int i = 0;
    int firstRun = 0;

    double firstPtLon = -1;
    double firstPtLat = -1;
    double lastPtLon = -1;
    double lastPtLat = -1;

    /* Iterating through the track and using the segments */
    void *elemTr;
    ListIterator iterTr = createIterator(tr->segments);
    while ((elemTr = nextElement(&iterTr)) != NULL) {
        TrackSegment* tmpTrackSegmenttData = (TrackSegment *)elemTr;
        int numPts = getLength(tmpTrackSegmenttData->waypoints);
        double coordsArray[2][numPts];

        /* Iterating through the waypoints and storing them in an array */
        i = 0;
        void *elemWp;
        ListIterator iterWp = createIterator(tmpTrackSegmenttData->waypoints);
        while ((elemWp = nextElement(&iterWp)) != NULL) {
            Waypoint* tmpWaypointData = (Waypoint *)elemWp;
            coordsArray[0][i] = tmpWaypointData->longitude;
            coordsArray[1][i] = tmpWaypointData->latitude;
            i++;
        }

        if (firstRun == 0) {
            firstPtLon = coordsArray[0][0];
            firstPtLat = coordsArray[1][0];
            firstRun = 1;
        }

        lastPtLon = coordsArray[0][numPts-1];
        lastPtLat = coordsArray[1][numPts-1];
    }

    double distance = getDistVal(firstPtLat, firstPtLon, lastPtLat, lastPtLon);
    if (fabs(distance*1000) <= delta) {
        return (true);
    } else {
        return (false);
    }
}

List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
    if (doc == NULL) {
        return (NULL);
    }
    int numRoutes = 0;
    List *routeList = initializeList(&routeToString, &deleteDummy, &compareRoutes);

    /* Iterating through the routes and storing them in an array */
    void *elemRt;
    ListIterator iterRt = createIterator(doc->routes);
    while ((elemRt = nextElement(&iterRt)) != NULL) {
        Route* tmpRouteData = (Route *)elemRt;

        int numPts = getLength(tmpRouteData->waypoints);
        int i = 0;
        double coordsArray[2][numPts];

        /* Iterate through waypoints */
        void *elemWp;
        ListIterator iterWp = createIterator(tmpRouteData->waypoints);
        while ((elemWp = nextElement(&iterWp)) != NULL) {
            Waypoint* tmpWaypointData = (Waypoint *)elemWp;
            coordsArray[0][i] = tmpWaypointData->longitude;
            coordsArray[1][i] = tmpWaypointData->latitude;
            i++;
        }

        /* Checking if distances fit the delta */
        if (fabs(coordsArray[1][0] - sourceLat)*1000 <= delta && fabs(coordsArray[0][0] - sourceLong)*1000 <= delta && fabs(coordsArray[1][numPts-1] - destLat)*1000 <= delta && fabs(coordsArray[0][numPts-1] - destLong)*1000 <= delta) {
            numRoutes++;
            insertBack(routeList, tmpRouteData);
        }
    }

    /* Returning at the end of the function */
    if (numRoutes == 0) {
        return (NULL);
    } else {
        return (routeList);
    }
}

List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta) {
    if (doc == NULL) {
        return (NULL);
    }
    int numTracks = 0;
    List *trackList = initializeList(&trackToString, &deleteDummy, &compareTracks);

    /* Iterating through the tracks and storing them in an array */
    void *elemTr;
    ListIterator iterTr = createIterator(doc->tracks);
    while ((elemTr = nextElement(&iterTr)) != NULL) {
        Track* tmpTrackData = (Track *)elemTr;  
        int firstRun = 0;

        double firstPtLon = -1;
        double firstPtLat = -1;
        double lastPtLon = -1;
        double lastPtLat = -1;

        /* Iterate through the segments */
        void *elemSeg;
        ListIterator iterSeg = createIterator(tmpTrackData->segments);
        while ((elemSeg = nextElement(&iterSeg)) != NULL) {
            TrackSegment* tmpTrackSegmentData = (TrackSegment *)elemSeg;
            int numPts = getLength(tmpTrackSegmentData->waypoints);
            int i = 0;
            double coordsArray[2][numPts];

            /* Iterate through waypoints */
            void *elemWp;
            ListIterator iterWp = createIterator(tmpTrackSegmentData->waypoints);
            while ((elemWp = nextElement(&iterWp)) != NULL) {
                Waypoint* tmpWaypointData = (Waypoint *)elemWp;
                coordsArray[0][i] = tmpWaypointData->longitude;
                coordsArray[1][i] = tmpWaypointData->latitude;
                i++;
            }

            if (firstRun == 0) {
                firstPtLon = coordsArray[0][0];
                firstPtLat = coordsArray[1][0];
                firstRun = 1;
            }

            lastPtLon = coordsArray[0][numPts-1];
            lastPtLat = coordsArray[1][numPts-1];   
        }

        if (fabs(firstPtLat - sourceLat)*1000 <= delta && fabs(firstPtLon - sourceLong)*1000 <= delta && fabs(lastPtLat - destLat)*1000 <= delta && fabs(lastPtLon - destLong)*1000 <= delta) {
            numTracks++;
            insertBack(trackList, tmpTrackData);
        }
    }

    /* Returning at the end of the function */
    if (numTracks == 0) {
        return (NULL);
    } else {
        return (trackList);
    }
}

char* routeToJSON(const Route *rt) {
    char *theString = malloc(sizeof(char) * 8192);
    if (rt == NULL) {
        strcpy(theString, "{}");
        return (theString);
    }

    char routeName[1024] = "";
    int numPoints = 0;
    double routeLen = 0;
    char loopStat[16] = "";

    /* Setting up routeName for the JSON sprintf */
    if (strlen(rt->name) == 0) {
        strcpy(routeName, "None");    
    } else {
        strcpy(routeName, rt->name);
    }

    /* Setting up numPoints for the JSON sprintf */
    numPoints = getLength(rt->waypoints);

    /* Setting up numPoints for the JSON sprintf */
    routeLen = round10(getRouteLen(rt));

    /* Setting up loopStat for the JSON sprintf */
    if (isLoopRoute(rt, 10) == true) {
        strcpy(loopStat, "TRUE");
    } else {
        strcpy(loopStat, "FALSE");
    }

    sprintf(theString, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%0.1f,\"loop\":\"%s\"}", routeName, numPoints, routeLen, loopStat);
    return(theString);
}

char* trackToJSON(const Track *tr) {
    char *theString = malloc(sizeof(char) * 8192);
    if (tr == NULL) {
        strcpy(theString, "{}");
        return (theString);
    }


    char routeName[1024] = "";
    int numPoints = 0;
    double routeLen = 0;
    char loopStat[16] = "";

    /* Setting up routeName for the JSON sprintf */
    if (strlen(tr->name) == 0) {
        strcpy(routeName, "None");    
    } else {
        strcpy(routeName, tr->name);
    }

    /* Setting up numPoints for the JSON sprintf */
    void *elem;
    ListIterator iter = createIterator(tr->segments);
    while ((elem = nextElement(&iter)) != NULL) {
        TrackSegment* tmpTrackSegData = (TrackSegment *)elem;
        numPoints += getLength(tmpTrackSegData->waypoints);
    }

    /* Setting up numPoints for the JSON sprintf */
    routeLen = round10(getTrackLen(tr));

    /* Setting up loopStat for the JSON sprintf */
    if (isLoopTrack(tr, 10) == true) {
        strcpy(loopStat, "TRUE");
    } else {
        strcpy(loopStat, "FALSE");
    }

    sprintf(theString, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%0.1f,\"loop\":\"%s\"}", routeName, numPoints, routeLen, loopStat);
    return(theString);
}

char* routeListToJSON(const List *list) {
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
        Route* tmpRouteData = (Route *)elem;
        char *tempString = routeToJSON(tmpRouteData);
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

char* trackListToJSON(const List *list) {
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
        Track* tmpTrackData = (Track *)elem;
        char *tempString = trackToJSON(tmpTrackData);
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

char* GPXtoJSON(const GPXdoc* gpx) {
    char *theString = malloc(sizeof(char) * 8192);
    strcpy(theString, "");
    if (gpx == NULL) {
        strcpy(theString, "{}");
        return (theString);
    }

    sprintf(theString + strlen(theString), "{\"version\":%0.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", gpx->version, gpx->creator, getLength(gpx->waypoints), getLength(gpx->routes), getLength(gpx->tracks));

    return(theString);
}

void addWaypoint(Route *rt, Waypoint *pt) {
    if (rt == NULL || pt == NULL) {
        return;
    }
    insertBack(rt->waypoints, pt);
}

void addRoute(GPXdoc* doc, Route* rt) {
    if (doc == NULL || rt == NULL) {
        return;
    }
    insertBack(doc->routes, rt);
}

GPXdoc* JSONtoGPX(const char* gpxString) {
    if (gpxString == NULL) {
        return(NULL);
    }

    char theString[8196] = "";
    strcpy(theString, gpxString);
    const char s[6] = ",{}:\"";
    char *token;
    GPXdoc *gdoc = (GPXdoc*)malloc(sizeof(GPXdoc));

    /* Getting namespace */
    strcpy(gdoc->namespace, "http://www.topografix.com/GPX/1/1");

    /* Getting junk */
    token = strtok(theString, s);

    /* Getting version */
    token = strtok(NULL, s);
    gdoc->version = atof(token);

    /* Getting junk */
    token = strtok(NULL, s);

    /* Getting creator */
    token = strtok(NULL, s);
    char *temp = (char*)malloc(sizeof(char) * 1024);
    strcpy(temp, token);
    gdoc->creator = temp;

    /* Initializing waypoint list */
    List *waypointList = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    gdoc->waypoints = waypointList;

    /* Initializing routes list */
    List *routeList = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    gdoc->routes = routeList;

    /* Initializing tracks list */
    List *trackList = initializeList(&trackToString, &deleteTrack, &compareTracks);
    gdoc->tracks = trackList;

    return (gdoc);
}

Waypoint* JSONtoWaypoint(const char* gpxString) {
    if (gpxString == NULL) {
        return(NULL);
    }

    char theString[8196] = "";
    strcpy(theString, gpxString);
    const char s[6] = ",{}:\"";
    char *token;
    Waypoint *wp = (Waypoint*)malloc(sizeof(Waypoint));

    /* Getting junk */
    token = strtok(theString, s);

    /* Getting lat */
    token = strtok(NULL, s);
    wp->latitude = atof(token);

    /* Getting junk */
    token = strtok(NULL, s);

    /* Getting lon */
    token = strtok(NULL, s);
    wp->longitude = atof(token);
    
    /* Initializing name */
    char *temp = (char*)malloc(sizeof(char) * 1024);
    strcpy(temp, "");
    wp->name = temp;

    /* Initializing otherData list */
    List *otherDataList = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    wp->otherData = otherDataList;

    return (wp);
}

Route* JSONtoRoute(const char* gpxString) {
    if (gpxString == NULL) {
        return(NULL);
    }
    char theString[8196] = "";
    strcpy(theString, gpxString);
    const char s[6] = ",{}:\"";
    char *token;
    char *temp = (char*)malloc(sizeof(char) * 1024);
    Route *rt = (Route*)malloc(sizeof(Route));

    /* Getting junk */
    token = strtok(theString, s);

    /* Getting name */
    token = strtok(NULL, s);
    if (token == NULL) {
        strcpy(temp, "");
    } else {
        strcpy(temp, token);
    }
    rt->name = temp;

    /* Initializing waypoint list */
    List *waypointList = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    rt->waypoints = waypointList;

    /* Initializing otherData list */
    List *otherDataList = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    rt->otherData = otherDataList;

    return (rt);
}