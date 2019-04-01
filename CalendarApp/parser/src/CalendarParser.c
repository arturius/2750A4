/*  CalanderParser.c
    Function file that parses a text input to valid calander objects for CIS 2750.
    By: James Arthur Anderson
    Student ID #: 1013076
    Date: January. 21, 2019
*/
#include"CalendarParser.h"
#include"HelperFunctions.h"
/** Function to create a Calendar object based on the contents of an iCalendar file.
 *@pre File name cannot be an empty string or NULL.  File name must have the .ics extension.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid calendar has been created, its address was stored in the variable obj, and OK was returned
		or 
		An error occurred, the calendar was not created, all temporary memory was freed, obj was set to NULL, and the 
		appropriate error code was returned
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param fileName - a string containing the name of the iCalendar file
 *@param a double pointer to a Calendar struct that needs to be allocated
**/
ICalErrorCode createCalendar(char* fileName, Calendar** obj){
    bool exitFlag= false;
    bool versionFlag = false;
    bool prodIDFlag = false;
    bool eventFlag = false;
    bool calEntered = false;
    char *readLine = NULL;
    char lName[200];
    int lineSize = 0;
    int deliminator = -1;
    FILE *fp = NULL;
    Event *tempEvent = NULL;
    Property *tempProp = NULL;
    ICalErrorCode code= OTHER_ERROR;
    //line unfolding and file validation
    //Opening temporary file to store unfolded line
    fp = fopen("tempUnfoldedIcalFile.txt","w+");
    if (fp == NULL){
        *obj = NULL;
        return OTHER_ERROR;
    }
    code = lineDefold(fileName,fp);
    
    if (code !=OK){
        if(fp){
            fclose(fp);
            remove("tempUnfoldedIcalFile.txt");
        }
        *obj = NULL;
        return code;
    }
    //calendar memory allocation
    *obj = malloc(sizeof(Calendar));
    if (*obj){
        (*obj)->properties = initializeList(&printProperty,&deleteProperty,&compareProperties);
        (*obj)->events = initializeList(&printEvent,&deleteEvent,&compareEvents);
        strcpy((*obj)->prodID,"");
    }else{
        //if memory alocation fails
        exitFlag = true;
        *obj = NULL;
        code = OTHER_ERROR;
    }

    //calendar creation
    if (!(*obj)){
        return INV_CAL;
    }
    while (!exitFlag){
        lineSize = gline(&readLine,fp);

        if(lineSize<0){
            if(lineSize == -1){
                code = INV_CAL;
                free(readLine);
                break;
            }else{
                code = OTHER_ERROR;
                free(readLine);
                break;
            }       
        }
        //removing the new line characchter if it exists
        if (lineSize-1>-1&&readLine[lineSize-1]== '\n'){
            readLine[lineSize-1]= '\0';
            lineSize--;
        }
        //finding the Name deliminator and if it does not exist returns invalid calendar
        deliminator = findDeliminator(readLine);
        if(deliminator<0){
                code = INV_CAL;
                free(readLine);
                break;
        }
        strncpy(lName,readLine,deliminator);
        lName[deliminator]= '\0';

       //checking for the end of the Event
        if (strcmp(lName,"END")== 0){
            exitFlag = true;
            if (strcmp(&(readLine[deliminator+1]),"VCALENDAR")==0){//any other end conditions makes the calendar invalid
                if (versionFlag&&prodIDFlag&&eventFlag){
                    code = OK;
                }else{
                    code = INV_CAL;
                }
                calEntered = false;
            }else{
                code = INV_CAL;
            }
        }else if(strcmp(lName,"PRODID")== 0&&calEntered){
            if(strcmp(&(readLine[deliminator+1]),"")==0){
                code = INV_PRODID;
                exitFlag = true;
            }

            if (!prodIDFlag){
                strncpy((*obj)->prodID,&(readLine[deliminator+1]),1000);
                prodIDFlag = true;
            }else{
                exitFlag = true;
                code = DUP_PRODID;
            }
        }else if(strcmp(lName,"VERSION")== 0&&calEntered){
            if(strcmp(&(readLine[deliminator+1]),"")==0){
                exitFlag = true;
                code = INV_VER;
            }else if(atof(&(readLine[deliminator+1]))==0.0){
                exitFlag = true;
                code = INV_VER;
            }
            if (!versionFlag){
               (*obj)->version = atof(&(readLine[deliminator+1]));
                versionFlag = true;
            }else{
                exitFlag = true;
                code = DUP_VER;
            }
        }else if(strcmp(lName,"BEGIN")== 0){
             if (strcmp(&(readLine[deliminator+1]),"VEVENT")==0&&calEntered){//might want tomake this safe
                tempEvent = newEvent();
                code = createEvent(fp,&tempEvent);
                if (code == OK){
                    insertBack((*obj)-> events,(void*)tempEvent);
                    eventFlag = true;
                    tempEvent = NULL;
                }else{
                    deleteEvent((void*)tempEvent);
                    exitFlag = true;
                }
            }else if(strcmp(&(readLine[deliminator+1]),"VCALENDAR")==0){
                calEntered = true;
            }else{
                code = INV_CAL;
            }
        }else if(calEntered){
            if ((*obj)->properties&&!tempProp){
                code = createProperty(readLine,&tempProp);
                if (code == OK){
                    insertBack((*obj)->properties,(void*)tempProp);
                    tempProp = NULL;
                }else{
                    deleteProperty((void*)tempProp);
                    if(code == WRITE_ERROR){
                        code = INV_CAL;
                    }
                    exitFlag = true;
                }
            }
        }
        free(readLine);
    }
    if(code!=OK){

        deleteCalendar((void*)*obj);
        *obj = NULL;
    }

    fclose(fp);
    remove("tempUnfoldedIcalFile.txt");

    return code;
}


/** Function to delete all calendar content and free all the memory.
 *@pre Calendar object exists, is not null, and has not been freed
 *@post Calendar object had been freed
 *@return none
 *@param obj - a pointer to a Calendar struct
**/
void deleteCalendar(Calendar* obj){
    if (obj){
        if(obj->events){
            freeList(obj->events);
        }
        if(obj->properties){
            freeList(obj->properties);
        }
        free(obj);
    }
}


/** Function to create a string representation of a Calendar object.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a string representing the Calndar contents has been created
 *@return a string contaning a humanly readable representation of a Calendar object
 *@param obj - a pointer to a Calendar struct
**/
char* printCalendar(const Calendar* obj){
    char* eventStr = NULL;
    int strsLen =0;
    char* properties =NULL;
    char* events = NULL;
    char version[200];
    if (!obj){
        return NULL;
    }
    sprintf(version," version ='%f'",obj->version);
    events = toString(obj->events);
    properties = toString(obj->properties);
    strsLen = strlen("Event;")+strlen(version);
    strsLen = strlen("'prodID:'")+strlen(obj->prodID);
    strsLen += strlen("'\nEvents:\n")+strlen(events);
    strsLen += strlen("Properties:\n")+strlen(properties)+50;
    eventStr = malloc(sizeof(char)*strsLen);
    if(eventStr){
        strcpy(eventStr,"Event; UID:'");
        strcpy(eventStr,version);
        strcat(eventStr,"'prodID:'");
        strcat(eventStr,obj->prodID); 
        strcat(eventStr,"Events:\n");
        strcat(eventStr,events);
        strcat(eventStr,"Properties:\n");
        strcat(eventStr,properties);
    }

    free(events);
    free(properties);
    return eventStr;
} 


/** Function to "convert" the ICalErrorCode into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code by indexing into 
          the descr array using rhe error code enum value as an index
 *@param err - an error code
**/
char* printError(ICalErrorCode err){
    int len =0;
    char *str;
    switch(err){
        case OK:
            len = strlen("Ok")+1;
            str = malloc(sizeof(char)*len);
            strcpy(str,"Ok");
            break;
        case INV_FILE:
            len = strlen("Invalid File")+1;
            str = malloc(sizeof(char)*len);
            strcpy(str,"Invalid File");
            break;
        case INV_CAL:
            len = strlen("Invalid Calendar")+1;
            str = malloc(sizeof(char)*len);
            strcpy(str,"Invalid Calendar");
            break;
        case INV_VER:
            len = strlen("Invalid version")+1;
            str = malloc(sizeof(char)*len);
            strcpy(str,"Invalid version");
            break;
        case DUP_VER:
            len = strlen("Duplicate version")+1;
            str = malloc(sizeof(char)*len);
            strcpy(str,"Duplicate version");
            break;
        case INV_PRODID:
            len = strlen("Invalid product ID")+1;
            str = malloc(sizeof(char)*len);
            strcpy(str,"Invalid product ID");
            break;
        case DUP_PRODID:
            len = strlen("Duplicate product ID")+1;
            str = malloc(sizeof(char)*len);
            strcpy(str,"Duplicate product ID");
            break;
        case INV_EVENT:
            len = strlen("Invalid event")+1;
            str = malloc(sizeof(char)*len);
            strcpy(str,"Invalid event");
            break;
        case INV_DT:
            len = strlen("Invalid date time")+1;
            str = malloc(sizeof(char)*len);
            strcpy(str,"Invalid date time");
            break;
        case INV_ALARM:
            len = strlen("Invalid Alarm")+1;
            str = malloc(sizeof(char)*len);
            strcpy(str,"Invalid Alarm");
            break;
        case WRITE_ERROR:
            len = strlen("Write Error")+1;
            str = malloc(sizeof(char)*len);
            strcpy(str,"Write Error");
            break;
        case OTHER_ERROR:
            len = strlen("Other Error")+1;
            str = malloc(sizeof(char)*len);
            strcpy(str,"Other Error");
            break;
        default:
            str =NULL;
    }
    return str;
}


// ************* A2 functionality - MUST be implemented ***************

/** Function to writing a Calendar object into a file in iCalendar format.
 *@pre
    Calendar object exists, and is not NULL.
    fileName is not NULL, has the correct extension
 *@post Calendar has not been modified in any way, and a file representing the
       Calendar contents in iCalendar format has been created
 *@return the error code indicating success or the error encountered when traversing the Calendar
 *@param
    obj - a pointer to a Calendar struct
 	fileName - the name of the output file
 **/
ICalErrorCode writeCalendar(char* fileName, const Calendar* obj){
    int errFlag;
    FILE *fp;
    ListIterator eIt, aIt, pIt;//event alarm and propertie interators
    Event *tempEvent;
    Alarm *tempAlarm;
    Property *tempProp;
    //property validation
    if (!fileName || !obj){
        return WRITE_ERROR;
    }
    if(strcmp(fileName,"")==0){//just so thier are no blank files
        return WRITE_ERROR;
    }
    fp = fopen(fileName,"w");
    if (!fp){
        return WRITE_ERROR;
    }
    //Printing calendar
    errFlag = fprintf(fp,"BEGIN:VCALENDAR\r\n");
    errFlag = fprintf(fp,"VERSION:%f\r\n",obj->version);
    errFlag = fprintf(fp,"PRODID:%s\r\n",obj->prodID);
    eIt = createIterator(obj->events);
    while ((tempEvent = nextElement(&eIt))){
            errFlag = fprintf(fp,"BEGIN:VEVENT\r\n");
            errFlag = fprintf(fp,"UID:%s\r\n",tempEvent->UID);
            if ((tempEvent->creationDateTime).UTC){
                errFlag = fprintf(fp,"DTSTAMP:%sT%sZ\r\n",(tempEvent->creationDateTime).date,(tempEvent->creationDateTime).time);
            }else{
                errFlag = fprintf(fp,"DTSTAMP:%sT%s\r\n",(tempEvent->creationDateTime).date,(tempEvent->creationDateTime).time);
            }
            if ((tempEvent->startDateTime).UTC){
                errFlag = fprintf(fp,"DTSTART:%sT%sZ\r\n",(tempEvent->startDateTime).date,(tempEvent->startDateTime).time);
            }else{
                errFlag = fprintf(fp,"DTSTART:%sT%s\r\n",(tempEvent->startDateTime).date,(tempEvent->startDateTime).time);
            }
            pIt = createIterator(tempEvent->properties);
            while ((tempProp = nextElement(&pIt))){
                errFlag = fprintf(fp,"%s:%s\r\n",tempProp->propName,tempProp->propDescr);
            }
            aIt = createIterator(tempEvent->alarms);
            while ((tempAlarm = nextElement(&aIt))){
                errFlag = fprintf(fp,"BEGIN:VALARM\r\n");
                errFlag = fprintf(fp,"ACTION:%s\r\n",tempAlarm->action);
                errFlag = fprintf(fp,"TRIGGER:%s\r\n",tempAlarm->trigger);
                pIt = createIterator(tempAlarm->properties);
                while ((tempProp = nextElement(&pIt))){
                    errFlag = fprintf(fp,"%s:%s\r\n",tempProp->propName,tempProp->propDescr);
                }
                errFlag = fprintf(fp,"END:VALARM\r\n");
            }
            errFlag = fprintf(fp,"END:VEVENT\r\n");
    }
    pIt = createIterator(obj->properties);
    while ((tempProp = nextElement(&pIt))){
        errFlag = fprintf(fp,"%s:%s\r\n",tempProp->propName,tempProp->propDescr);
    }
    errFlag = fprintf(fp,"END:VCALENDAR\r\n");
    if(errFlag < 0){
        return WRITE_ERROR;
    }
    fclose(fp);
    return OK;
}


/** Function to validating an existing a Calendar object
 *@pre Calendar object exists and is not NULL
 *@post Calendar has not been modified in any way
 *@return the error code indicating success or the error encountered when validating the calendar
 *@param obj - a pointer to a Calendar struct
 **/
ICalErrorCode validateCalendar(const Calendar* obj){
    //lists of all the valid properties per type
    const char *ALMPROP[] = {"AFTER THIS CAN ONLY OCCUR ONCE","DURATION","REPEAT","ATTACH",NULL};
    const char *CALPROP[] = {"AFTER THIS CAN ONLY OCCUR ONCE","METHOD","CALSCALE",NULL};
    const char *EVENTPROP[] = {"ATTACH","ATTENDEE","CATEGORIES","COMMENT","CONTACT"
                             ,"EXDATE","REQUEST-STATUS","RELATED-TO","RESOURCES","RDATE","RRULE","DTEND","DURATION",
                            "AFTER THIS CAN ONLY OCCUR ONCE",
                            "CLASS","CREATED","DESCRIPTION","GEO","LAST-MODIFIED","LOCATION","ORGANIZER"
                            ,"PRIORITY","SEQUENCE","STATUS","SUMMARY","TRANSP","URL","RECURRENCE-ID",NULL};
    ListIterator eIt, aIt, pIt;//event alarm and propertie interators
    Event *tempEvent;
    Alarm *tempAlarm;
    Property *tempProp;
    strNode *list = NULL;
    strNode *temp;
   //property validation
    if (!obj){
       return INV_CAL;
    }
    /*if(!obj->version){
        if (obj->version == 0.0){
printf("9");
            return INV_CAL;
        }
    }else{
printf("8");
        return INV_CAL;
    }*/
    if (obj->prodID){
        if (strcmp(obj->prodID,"")==0){
           return INV_CAL;
        }
        
      
        if (strlen(obj->prodID)>1000){
            return INV_CAL;
        }
    }else{
       return INV_CAL;
    }
    if (obj->properties){
        pIt = createIterator(obj->properties);
    }else{
        return INV_CAL;
    }
    
    while ((tempProp = nextElement(&pIt))){
        if (strcmp(tempProp->propDescr,"")!=0){
            if(!isValidProperty(tempProp->propName,CALPROP,&list)){
                deleteList(&list);
                return INV_CAL;
            }
        }else{
           deleteList(&list);
           return INV_CAL;
        }
    }
    deleteList(&list);
    if (obj->events){
        eIt = createIterator(obj->events);
    }else{
       return INV_CAL;
    }
    if (eIt.current){
        while ((tempEvent = nextElement(&eIt))){
                if(tempEvent->UID){
                    if (strcmp(tempEvent->UID,"")==0){
                        return INV_EVENT;
                    }
                    if (strlen(tempEvent->UID)>1000){
                        return INV_EVENT;
                    }
                }else{
                    return INV_EVENT;
                }
                
                if (strcmp((tempEvent->creationDateTime).date,"")==0||strcmp((tempEvent->creationDateTime).time,"")==0
                        ||strlen((tempEvent->creationDateTime).date)>9||strlen((tempEvent->creationDateTime).time)>7){
                    return INV_EVENT;
                }
                
                if (strcmp((tempEvent->startDateTime).date,"")==0||strcmp((tempEvent->startDateTime).time,"")==0
                        ||strlen((tempEvent->startDateTime).date)>9||strlen((tempEvent->startDateTime).time)>7){
                    
                    return INV_EVENT;
                }
                
                if (tempEvent->properties){
                    pIt = createIterator(tempEvent->properties);
                }else{
                    return INV_EVENT;
                }
                while ((tempProp = nextElement(&pIt))){
                    if (strcmp(tempProp->propDescr,"")!=0){
                        if(!isValidProperty(tempProp->propName,EVENTPROP,&list)){
                            deleteList(&list);
                            return INV_EVENT;
                        }
                        if (strcmp(tempProp->propName,"DTEND")==0){
                            temp = newStrNode("DURATION");
                            add2StrNode(&list,temp);
                        }else if(strcmp(tempProp->propName,"DURATION")==0){
                            temp = newStrNode("DTEND");
                            add2StrNode(&list,temp);

                        }

                    }else{
                        deleteList(&list);
                        return INV_EVENT;
                    }
                }
                deleteList(&list);
                if (tempEvent->alarms){
                    aIt = createIterator(tempEvent->alarms);
                }else{
                    return INV_EVENT;
                }
                while ((tempAlarm = nextElement(&aIt))){
                    if(tempAlarm->action){
                        if (strcmp(tempAlarm->action,"")==0){         
                            return INV_ALARM;
                        }
                        if (strlen(tempAlarm->action)>200){
                            return INV_ALARM;
                        }
                    }else{
                        return INV_ALARM;
                    }
                    if(tempAlarm->trigger){
                         if (strcmp(tempAlarm->trigger,"")==0){  
                            return INV_ALARM;
                        }                        
                    }else{
                       return INV_ALARM;
                    }
                    if (tempAlarm->properties){
                        pIt = createIterator(tempAlarm->properties);
                    }else{ 
                        return INV_ALARM;
                    }
                    while ((tempProp = nextElement(&pIt))){
                        if (strcmp(tempProp->propDescr,"")!=0){
                            if(!isValidProperty(tempProp->propName,ALMPROP,&list)){
                                deleteList(&list);
                                return INV_ALARM;
                            }
                        }else{
                            deleteList(&list);
                            return INV_ALARM;
                        }
                    }
                    //check for mutuality of Duration and repeat
                    if (!isValidProperty("DURATION",ALMPROP,&list)){
                        //Duration Exists in the list
                        if (isValidProperty("REPEAT",ALMPROP,&list)){
                            //REPEAT doesn't Occur
                            deleteList(&list);
                            return INV_ALARM;
                        }
                    }else{
                        //Duration dosn't exist 
                        if (!isValidProperty("REPEAT",ALMPROP,&list)){
                            //REPEAT does Occur
                            deleteList(&list);
                            return INV_ALARM;
                        }
                   }
                    deleteList(&list);
                }
            
        }
    }else{
       return INV_CAL;
    }
    deleteList(&list);
    return OK;
}

// *******************JSON Conversion*********************//

/** Function to converting a DateTime into a JSON string
 *@pre N/A
 *@post DateTime has not been modified in any way
 *@return A string in JSON format
 *@param prop - a DateTime struct
 **/
char* dtToJSON(DateTime prop){
    char *jsonFrm = NULL;
    char t[] = "true";
    char f[] = "false";
    int strLen;
    if (!prop.date||!prop.time){
        return "{}";
    }
    strLen = strlen("{\"date\":\"\",\"time\":\"\",\"isUTC\":}");
    strLen += strlen(prop.date) + strlen(prop.time);
    if (prop.UTC){
        strLen += strlen(t);
    }else{
        strLen += strlen(f);
    }
    strLen++;
    jsonFrm = malloc(strLen);
    if (prop.UTC){
        sprintf(jsonFrm,"{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":%s}",prop.date,prop.time,t);
    }else{
        sprintf(jsonFrm,"{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":%s}",prop.date,prop.time,f);
    }
    return jsonFrm;
}
/** Function to converting an Alarm into a JSON string
 *@pre Alarm is not NULL
 *@post Alarm has not been modified in any way
 *@return A string in JSON format
 *@param alarm - a pointer to an Alarm struct
 **/
char* AlarmToJSON(const Alarm* alarm){
    char *jsonFrm = NULL;
    int strLen;
    if (!alarm->action||!alarm->trigger){
        return "{}";
    }
    strLen = strlen("{\"action\":,\"\"trigger\":\"\"}");
    strLen += strlen(alarm->action)+strlen(alarm->trigger);
    strLen++;
    jsonFrm = malloc(strLen);
    sprintf(jsonFrm,"{\"action\":\"%s\",\"trigger\":\"%s\"}",alarm->action,alarm->trigger);
    return jsonFrm;
}

/** Function to converting an Event list into a JSON string
 *@pre Alarm list is not NULL
 *@post Alarm list has not been modified in any way
 *@return A string in JSON format
 *@param alarmList - a pointer to an Event list
 **/
char* alarmListToJSON(const List* alarmList){
    char *jsonFrm = NULL;
    int strLen = 3;
    char *tempAlmJson;
    bool first = true;
    Alarm *tempAlarm;
    ListIterator aIt;

    if (!alarmList){
        return "[]";
    }
    jsonFrm = malloc(strLen);
    strcpy (jsonFrm,"[\0");
    aIt.current = NULL;
    aIt = createIterator((List*)alarmList);
    if (aIt.current){
        while ((tempAlarm = nextElement(&aIt))){
            tempAlmJson = AlarmToJSON(tempAlarm);
            if (!first){
                strLen ++;//handles comma
            }
            strLen += strlen(tempAlmJson);
            jsonFrm = realloc(jsonFrm,strLen);
            if (!first){
                strcat(jsonFrm,",");
            }
            strcat(jsonFrm,tempAlmJson);
            free(tempAlmJson);
            first = false;
        }
    }
    strcat(jsonFrm,"]\0");
    return jsonFrm;}

/** Function to converting an Property into a JSON string
 *@pre Property is not NULL
 *@post Property has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to an Property struct
 **/
char* propToJSON(const Property* prop){
    char *jsonFrm = NULL;
    int strLen;
    if (!prop->propName||!prop->propDescr){
        return "{}";
    }
    strLen = strlen("{\"propName\":,\"\"propsDescr\":\"\"}");
    strLen += strlen(prop->propName)+strlen(prop->propDescr);
    strLen++;
    jsonFrm = malloc(strLen);
    sprintf(jsonFrm,"{\"propName\":\"%s\",\"propDescr\":\"%s\"}",prop->propName,prop->propDescr);
    return jsonFrm;
}

/** Function to converting an Property list into a JSON string
 *@pre Property list is not NULL
 *@post Property list has not been modified in any way
 *@return A string in JSON format
 *@param propList - a pointer to an Property list
 **/
char* propListToJSON(const List* propList){
    char *jsonFrm = NULL;
    int strLen = 3;
    char *tempPrpJson;
    bool first = true;
    Property *tempProp;
    ListIterator pIt;

    if (!propList){
        return "[]";
    }
    jsonFrm = malloc(strLen);
    strcpy (jsonFrm,"[\0");
    pIt.current = NULL;
    pIt = createIterator((List*)propList);
    if (pIt.current){
        while ((tempProp = nextElement(&pIt))){
            tempPrpJson = propToJSON(tempProp);
            if (!first){
                strLen ++;//handles comma
            }
            strLen += strlen(tempPrpJson);
            jsonFrm = realloc(jsonFrm,strLen);
            if (!first){
                strcat(jsonFrm,",");
            }
            strcat(jsonFrm,tempPrpJson);
            free(tempPrpJson);
            first = false;
        }
    }
    strcat(jsonFrm,"]\0");
    return jsonFrm;
}
/** Function to converting an Event into a JSON string
 *@pre Event is not NULL
 *@post Event has not been modified in any way
 *@return A string in JSON format
 *@param event - a pointer to an Event struct
 **/
char* eventToJSON(const Event* event){
    char *jsonFrm = NULL;
    int strLen;
    char *tempDtJson;
    char intBuff[40];
    Property *tempProp;
    ListIterator pIt;
    bool sumExist= false;
    if (!event){
        return"{}";
    }

    pIt = createIterator(event->properties);
    while ((tempProp = nextElement(&pIt))){
        if(strcmp(tempProp->propName,"SUMMARY")==0){
            sumExist = true;
            break;
        }
    }
    tempDtJson = dtToJSON(event->startDateTime);
    int plLen = 3;
    plLen += getLength(event->properties);
    int alLen;
    alLen = getLength(event->alarms);
    strLen = strlen("{\"startDT\":,\"numProps\":,\"numAlarms\":,\"summary\":\"\"}");
    strLen += strlen(tempDtJson) + sprintf(intBuff,"%d%d",plLen,alLen);
    if (sumExist){
        strLen += strlen(tempProp->propDescr);
    }
    strLen++;
    jsonFrm = malloc(strLen);
    if (sumExist){
        sprintf(jsonFrm,"{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\"}",tempDtJson,plLen,alLen,tempProp->propDescr);
    }else{
        sprintf(jsonFrm,"{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"\"}",tempDtJson,plLen,alLen);
    }
    free(tempDtJson);
    return jsonFrm;
}

/** Function to converting an Event list into a JSON string
 *@pre Event list is not NULL
 *@post Event list has not been modified in any way
 *@return A string in JSON format
 *@param eventList - a pointer to an Event list
 **/
char* eventListToJSON(const List* eventList){
    char *jsonFrm = NULL;
    int strLen = 3;
    char *tempEvtJson;
    bool first = true;
    Event *tempEvent;
    ListIterator eIt;

    if (!eventList){
        return "[]";
    }
    jsonFrm = malloc(strLen);
    strcpy (jsonFrm,"[\0");
    eIt.current = NULL;
    eIt = createIterator((List*)eventList);
    if (eIt.current){
        while ((tempEvent = nextElement(&eIt))){
            tempEvtJson = eventToJSON(tempEvent);
            if (!first){
                strLen ++;//handles comma
            }
            strLen += strlen(tempEvtJson);
            jsonFrm = realloc(jsonFrm,strLen);
            if (!first){
                strcat(jsonFrm,",");
            }
            strcat(jsonFrm,tempEvtJson);
            free(tempEvtJson);
            first = false;
        }
    }
    strcat(jsonFrm,"]\0");
    return jsonFrm;
}

/** Function to converting a Calendar into a JSON string
 *@pre Calendar is not NULL
 *@post Calendar has not been modified in any way
 *@return A string in JSON format
 *@param cal - a pointer to a Calendar struct
 **/
char* calendarToJSON(const Calendar* cal){
    char *jsonFrm = NULL;
    int strLen;
    char intBuff[80];
    if (!cal){
        return"{}";
    }
    int plLen = 2;
    plLen += getLength(cal->properties);
    int elLen;
    elLen = getLength(cal->events);
    strLen = strlen("{\"version\":,\"prodID\":\"\",\"numProps\":,\"numEvents\":}");
    strLen += strlen(cal->prodID) + sprintf(intBuff,"%d%d%d",plLen,elLen,(int)cal->version);
    strLen++;
    jsonFrm = malloc(strLen);
    sprintf(jsonFrm,"{\"version\":%d,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%d}",(int)cal->version,cal->prodID,plLen,elLen);
    return jsonFrm;
}

/** Function to converting a JSON string into a Calendar struct
 *@pre JSON string is not NULL
 *@post String has not been modified in any way
 *@return A newly allocated and partially initialized Calendar struct
 *@param str - a pointer to a string
 **/
Calendar* JSONtoCalendar(const char* str){
    Calendar* retVal = NULL;
    if (!str){
        return NULL;
    }
    retVal = malloc(sizeof(Calendar));
    if (retVal){
        retVal->properties = initializeList(&printProperty,&deleteProperty,&compareProperties);
        retVal->events = initializeList(&printEvent,&deleteEvent,&compareEvents);
    }else{
        //if memory alocation fails
        return NULL;
    }
    sscanf(str,"{\"version\":%f,\"prodID\":\"%999[^\"]\"}",&(retVal->version),retVal->prodID); 
    return retVal;
}

/** Function to converting a JSON string into an Event struct
 *@pre JSON string is not NULL
 *@post String has not been modified in any way
 *@return A newly allocated and partially initialized Event struct
 *@param str - a pointer to a string
 **/
Event* JSONtoEvent(const char* str){
    Event* retVal = NULL;
    char summary[1000];
    DateTime start, create;
    if (!str){
        return NULL;
    }
    char boolVal[10];
    strcpy(summary,"");
    retVal = newEvent();
    sscanf(str,"{\"UID\":\"%999[^\"]\",\"startDate\":\"%8[^\"]\",\"startTime\":\"%6[^\"]\",\"UTC\":\"%9[^\"]\",\"creationDate\":\"%8[^\"]\",\"creationTime\":\"%6[^\"]\",\"sum\":\"%999[^\"]\"}",retVal->UID,start.date,start.time,boolVal,create.date,create.time,summary);
    create.UTC = false;
    if (strcmp(boolVal,"true")==0){
        start.UTC = true;
    }else{
        start.UTC = false;
    }
    if (strcmp(summary,"")!=0){
        Property *tempProp= NULL;
        tempProp = malloc(sizeof(Property)+((strlen(summary)+1)*sizeof(char)));
        strcpy(tempProp->propName,"SUMMARY");
        strcpy(tempProp->propDescr,summary);
        insertBack(retVal->properties,tempProp);
    }
   
    retVal->creationDateTime = create;
    retVal->startDateTime = start;
    return retVal;
}

/** Function to adding an Event struct to an ixisting Calendar struct
 *@pre arguments are not NULL
 *@post The new event has been added to the calendar's events list
 *@return N/A
 *@param cal - a Calendar struct
 *@param toBeAdded - an Event struct
 **/
void addEvent(Calendar* cal, Event* toBeAdded){
    if (!cal || !toBeAdded){
        return;
    }
    insertBack(cal->events,toBeAdded);
    return;
}

// *********************************************************************


// ************* List helper functions - MUST be implemented *************** 
void deleteEvent(void* toBeDeleted){
    if (toBeDeleted){
        //deleteDate((void*)(&(((Event *)toBeDeleted)->creationDateTime)));
        //deleteDate((void*)(&(((Event *)toBeDeleted)->startDateTime)));
        if(((Event *)toBeDeleted)->alarms){
            freeList(((Event *)toBeDeleted)->alarms);
        }
        if(((Event *)toBeDeleted)->properties){
            freeList(((Event *)toBeDeleted)->properties);
        }
        free(toBeDeleted);
    }
}
int compareEvents(const void* first, const void* second){
    char *s1;
    char *s2;
    int reval;
    s1 = printEvent((void *)first);
    s2 = printEvent((void *)second);
    reval = strcmp(s1,s1);
    free(s1);
    free(s2);
    return reval;
}
char* printEvent(void* toBePrinted){
    char* eventStr = NULL;
    int strsLen =0;
    char* properties =NULL;
    char* alarms = NULL;
    char *startDate = NULL;
    char *createDate = NULL;
    startDate = printDate((void *)(&((Event *)toBePrinted)->startDateTime));
    createDate = printDate((void *)(&((Event *)toBePrinted)->creationDateTime));
    alarms = toString(((Event *)toBePrinted)->alarms);
    properties = toString(((Event *)toBePrinted)->properties);
    strsLen = strlen("Event; UID:'")+strlen(((Event *)toBePrinted)->UID);
    strsLen += strlen("'\nCreation ")+strlen(createDate);
    strsLen += strlen("\nStart ")+strlen(startDate);
    strsLen += strlen("\nAlarms:\n")+strlen(alarms);
    strsLen += strlen("Properties:\n")+strlen(properties)+1;
    eventStr = malloc(sizeof(char)*strsLen);
    if(eventStr){
        strcpy(eventStr,"Event; UID:'");
        strcat(eventStr,((Event *)toBePrinted)->UID);
        strcat(eventStr,"'\nCreation ");
        strcat(eventStr,createDate);
        strcat(eventStr,"\nStart ");
        strcat(eventStr,startDate);
        strcat(eventStr,"\nAlarms:\n");
        strcat(eventStr,alarms);
        strcat(eventStr,"Properties:\n");
        strcat(eventStr,properties);
    }
    free(startDate);
    free(createDate);
    free(alarms);
    free(properties);
    return eventStr;
}

void deleteAlarm(void* toBeDeleted){
    if (toBeDeleted){
        if(((Alarm *)toBeDeleted)->trigger){
            free(((Alarm *)toBeDeleted)->trigger);
        }
        if(((Alarm *)toBeDeleted)->properties){
            freeList(((Alarm *)toBeDeleted)->properties);
        }
        free(toBeDeleted);
    }
}
int compareAlarms(const void* first, const void* second){
    char *s1;
    char *s2;
    int reval;
    s1 = printAlarm((void *)first);
    s2 = printAlarm((void *)second);
    reval = strcmp(s1,s1);
    free(s1);
    free(s2);
    return reval;
}
char* printAlarm(void* toBePrinted){
    char* alarmStr = NULL;
    int strsLen =0;
    char* properties =NULL;
    properties = toString(((Alarm *)toBePrinted)->properties);
    strsLen = strlen("Alarm; Action:'")+strlen(((Alarm *)toBePrinted)->action);
    strsLen += strlen("' Trigger:'")+strlen(((Alarm *)toBePrinted)->trigger);
    strsLen += strlen("' Properties:\n")+strlen(properties)+1;
    alarmStr = malloc(sizeof(char)*strsLen);
    if(alarmStr){
        strcpy(alarmStr,"Alarm; Action:'");
        strcat(alarmStr,((Alarm *)toBePrinted)->action);
        strcat(alarmStr,"' Trigger:");
        strcat(alarmStr,((Alarm *)toBePrinted)->trigger);
        strcat(alarmStr,"' Properties:\n");
        strcat(alarmStr,properties);
    }
    free(properties);
    return alarmStr;
}

void deleteProperty(void* toBeDeleted){
    if(toBeDeleted){
        free(toBeDeleted);
    }
}
int compareProperties(const void* first, const void* second){
    char *s1;
    char *s2;
    int reval;
    s1 = printProperty((void *)first);
    s2 = printProperty((void *)second);
    reval = strcmp(s1,s1);
    free(s1);
    free(s2);
    return reval;
}
char* printProperty(void* toBePrinted){
    char* propStr = NULL;
    int strsLen =0;
    strsLen = strlen("Property; Name:'")+strlen(((Property *)toBePrinted)->propName);
    strsLen += strlen("' Description:")+strlen(((Property *)toBePrinted)->propDescr)+1;
    propStr = malloc(sizeof(char)*strsLen);
    if(propStr){
        strcpy(propStr,"Property; Name:'");
        strcat(propStr,((Property *)toBePrinted)->propName);
        strcat(propStr,"' Description:");
        strcat(propStr,((Property *)toBePrinted)->propDescr);
    }
    return propStr;
}

void deleteDate(void* toBeDeleted){
    if(toBeDeleted){
        free(toBeDeleted);
    }
}
int compareDates(const void* first, const void* second){
    char *s1;
    char *s2;
    int reval;
    s1 = printDate((void *)first);
    s2 = printDate((void *)second);
    reval = strcmp(s1,s1);
    free(s1);
    free(s2);
    return reval;
}
char* printDate(void* toBePrinted){
    char* dateStr = NULL;
    int strsLen =0;
    strsLen = strlen("Date Time; Date:'")+strlen(((DateTime *)toBePrinted)->date);
    strsLen += strlen("' Time:'")+strlen(((DateTime *)toBePrinted)->time)+50;
    dateStr = malloc(sizeof(char)*strsLen);
    if(dateStr){
        strcpy(dateStr,"Date Time; Date:'");
        strcat(dateStr,((DateTime *)toBePrinted)->date);
        strcat(dateStr,"' Time:'");
        strcat(dateStr,((DateTime *)toBePrinted)->time);
        if(((DateTime *)toBePrinted)->UTC){
            strcat(dateStr,"' UTC = True");
        }else{
            strcat(dateStr,"' UTC = False");
        }
    }
    return dateStr;
}
// **************************************************************************
//Glue functions

char *calJSONFromFileName (char *fileName){
    ICalErrorCode code;
    Calendar *tempCal;
    char *returnVal;
    code = createCalendar(fileName, &tempCal);
    if (code != OK){
        return printError(code);
//        return fileName;
    }else{
        returnVal = calendarToJSON(tempCal);
        deleteCalendar(tempCal);
        return returnVal;
    }
}

char *eventListJSONFromFileName (char *fileName){
    ICalErrorCode code;
    Calendar *tempCal;
    char *returnVal;
    code = createCalendar(fileName, &tempCal);
    if (code != OK){
        return printError(code);
//        return fileName;
    }else{
        returnVal = eventListToJSON(tempCal->events);
        deleteCalendar(tempCal);
        return returnVal;
    }
}

char *propListJSONFromFileName (char *fileName,char *eventNum){
    ICalErrorCode code;
    Calendar *tempCal;
    char *returnVal;
    Event *tempEvent;
    ListIterator eIt;
    int evtNum =0;
    int i = 0;
    evtNum = atoi(eventNum)-1;
    code = createCalendar(fileName, &tempCal);
    if (code != OK){
        return printError(code);
    }else{
        eIt = createIterator(tempCal->events);
        if (eIt.current){
            while ((tempEvent = nextElement(&eIt))&& i<evtNum){
                i++;
            }
            if (tempEvent){
                returnVal = propListToJSON(tempEvent->properties);
            }else{
                returnVal = malloc(strlen("[]")+1);
                strcpy(returnVal,"[]");
            }
        }else{
            returnVal = malloc(strlen("[]")+1);
            strcpy(returnVal,"[]");
        }
        deleteCalendar(tempCal);
        return returnVal;
    }
    return NULL;
}

char *alarmListJSONFromFileName (char *fileName, char *eventNum){
    ICalErrorCode code;
    Calendar *tempCal;
    char *returnVal;
    Event *tempEvent;
    ListIterator eIt;
    int evtNum =0;
    int i = 0;
    evtNum = atoi(eventNum)-1;
    code = createCalendar(fileName, &tempCal);
    if (code != OK){
        return printError(code);
    }else{
        eIt = createIterator(tempCal->events);
        if (eIt.current){
            while ((tempEvent = nextElement(&eIt))&& i<evtNum){
                i++;
            }
            if (tempEvent){
                returnVal = alarmListToJSON(tempEvent->alarms);
            }else{
                returnVal = malloc(strlen("[]")+1);
                strcpy(returnVal,"[]");
            }
        }else{
            returnVal = malloc(strlen("[]")+1);
            strcpy(returnVal,"[]");
        }
        deleteCalendar(tempCal);
        return returnVal;
    }
    return NULL;
}

char *createCalFileFromJSON(char *fileName, char *calJSON, char *eventJSON){
    ICalErrorCode code;
    Calendar *tempCal;
    Event *tempEvent;
    tempCal = JSONtoCalendar(calJSON);
//return printCalendar(tempCal);

    tempEvent = JSONtoEvent(eventJSON);
    addEvent(tempCal, tempEvent);
//    return printCalendar(tempCal);
    code = validateCalendar(tempCal);
    if (code == OK){
        writeCalendar(fileName, tempCal);
    }
    deleteCalendar(tempCal);
    return printError(code);
}

char *addEventToCalFileFromJSON(char *fileName, char *JSON){
    ICalErrorCode code;
    Calendar *tempCal;
    Event *tempEvent;
    code = createCalendar(fileName, &tempCal);
    if (code != OK){
        return printError(code);
    }else{
        tempEvent = JSONtoEvent(JSON);
 //       return printEvent(tempEvent);

        addEvent(tempCal, tempEvent);
        code = validateCalendar(tempCal);
        if (code == OK){
            writeCalendar(fileName, tempCal);
        }
        deleteCalendar(tempCal);
        return printError(code);
   //     return printEvent(tempEvent);
    }

}
