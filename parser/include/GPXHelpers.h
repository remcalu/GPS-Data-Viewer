#ifndef GPX_HELPERS_H
#define GPX_HELPERS_H

#include "GPXParser.h"

/* GPX Helper functions */
float getDistVal(double val1, double val2, double val3, double val4);
void deleteDummy(void* data);
char *validUploadedFile(char *fileName);
char *gpxToHTML(char *fileName);
char *rteToHTML(char *fileName);
char *trkToHTML(char *fileName);
char *ptsToJSON(const Waypoint *wpt, int index);
char *ptsListToJSON(const List *list, int location);
char *ptsToHTML(char *fileName, int location);
char *otherDataToJSON(const GPXData *od);
char *otherDataListToJSON(const List *list);
char *otherDataToHTML(char *fileName, int index);
char *changeRouteOrTrack(char *fileName, char *newName, int index);
char *addGPXToUploads(char *stringJSON, char *fileName);
char *addRouteToFile(char *fileName, char *waypointsJSON, char *routeName);
char *replaceSubString(char *string, char *toReplace, char *replacement);
char *findRoutePathsToJSON(char *fileArray, float latStart, float lonStart, float latEnd, float lonEnd, float delta);
char *findTrackPathsToJSON(char *fileArray, float latStart, float lonStart, float latEnd, float lonEnd, float delta);

#endif