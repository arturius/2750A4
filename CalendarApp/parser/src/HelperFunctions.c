/*  HelperFunctions.c
    Function file that helps parses a text input to specific calander objects for CIS 2750.
    By: James Arthur Anderson
    Student ID #: 1013076
    Date: January. 21, 2019
*/
#include<stdlib.h>
#include<string.h>
#include"CalendarParser.h"
#include"HelperFunctions.h"
#include"LinkedListAPI.h"

//Takes the name of a File and a empty file pointer and makes a new file wich has all of its lines unfolded
ICalErrorCode lineDefold(char *orignalFileName, FILE *unfoldedFile){
    FILE *foldfp = NULL;
    int errorFlag =0;
    int threeCount = 0;
    char threeBuffer[4];
    char charBuffer;
    bool rcFound = false;
    bool commentFlag = false;
    char previous;
    char *token = NULL;
    char *safeFileName;
   //check for file validity
if(orignalFileName){
        if(strcmp(orignalFileName,"")!=0){
            //check for .ics extention
            safeFileName = malloc((strlen(orignalFileName)+1)*sizeof(char));
            strcpy(safeFileName,orignalFileName);
            token = strtok(safeFileName,".");
            if(token){
                token = strtok(NULL,".");

                if (token){
                    if(strcmp(token,"ics")!=0){
                        free(safeFileName);
                        return INV_FILE;
                    }
                }else{
                    free(safeFileName);
                    return INV_FILE;
                }
            }else{
                free(safeFileName);
                return INV_FILE;
            }

            foldfp = fopen(orignalFileName,"r");
            free(safeFileName);
            if (foldfp == NULL){
                return INV_FILE;
            }
        }else{
            return INV_FILE;
        }
    }else{
        return INV_FILE;
    }
    charBuffer = getc (foldfp);
    if(charBuffer == ';'){
        commentFlag = true;
    }else{
        errorFlag = fputc(charBuffer,unfoldedFile);
    }
    while (!feof(foldfp)){
        charBuffer = getc (foldfp);
        if (charBuffer == '\n'){
            if(previous !='\r'&&!commentFlag){
                fclose(foldfp);
                return INV_FILE;
            }
        }
        if (charBuffer == '\r'){
            rcFound = true;
            commentFlag = false;
        }
        if (rcFound == true){
            threeBuffer[threeCount] = charBuffer;
            if (threeCount == 2){
                if ((threeBuffer[2] == ' '||threeBuffer[2] == '\t')&&threeBuffer[1]== '\n' &&threeBuffer[0]=='\r'){
                    //ingnores the CRLF and SPACE/HTAB
                }else if (threeBuffer[2] == ';'&&threeBuffer[1]== '\n' &&threeBuffer[0]=='\r'){
                    commentFlag = true;
                }else if(threeBuffer[2] == '\r'&&threeBuffer[1]== '\n' &&threeBuffer[0]=='\r'){
                    //Handling blank lines
                    threeCount = 1;
                    previous ='\r';
                    continue;
                }else if(threeBuffer[1]== '\n' &&threeBuffer[0]=='\r'){
                    //removing the carrige return to make later parsing easier
                    threeBuffer[0]=threeBuffer[1];
                    threeBuffer[1]=threeBuffer[2];
                    threeBuffer[2]='\0';
                    //printing the reaming chars to the new File
                    if(!commentFlag){
                        errorFlag = fputs(threeBuffer,unfoldedFile);
                    }
                }else{
                    printf("If your are seeing this something went worng in the file unfolder with this three buffer '%c''%c''%c'\n",threeBuffer[0],threeBuffer[1],threeBuffer[2]);
                }
                threeCount = 0;
                rcFound = false;
            }else{
                threeCount++;
            }
        }else{
            if(!commentFlag){
                errorFlag = fputc(charBuffer,unfoldedFile);
            }
        }
        if (errorFlag == EOF){
            fclose(foldfp); 
            return WRITE_ERROR;
        }
        previous = charBuffer;

   }


    errorFlag = fseek(unfoldedFile, 0,SEEK_SET);
    
    fclose(foldfp); 
    if (errorFlag != 0){
        return OTHER_ERROR;
    }
    return OK;
}

//returns a pointer to a newly alocated Event Struct
Event *newEvent(){
    Event *newEvent = NULL;
    newEvent = malloc(sizeof(Event));
    if (newEvent){
        newEvent->startDateTime.UTC = false;
        newEvent->creationDateTime.UTC = false;
        strcpy(newEvent->UID,"");
        newEvent->properties = initializeList(&printProperty,&deleteProperty,&compareProperties);
        newEvent->alarms = initializeList(&printAlarm,&deleteAlarm,&compareAlarms);
    }
    return newEvent;
}
//Takes a file pointer and event stuct and assigns poperties acordingly
ICalErrorCode createEvent(FILE *fp, Event **useEvent){
    bool exitFlag= false;
    bool uidFlag = false;
    bool createFlag = false;
    bool startFlag = false;
    char *readLine = NULL;
    char *ammendedDT = NULL;
    char lName[200];
    int lineSize = 0;
    int deliminator = -1;
    Alarm *tempAlarm = NULL;
    Property *tempProp = NULL;
    ICalErrorCode code= OTHER_ERROR;
    if (!(*useEvent)){
        return INV_EVENT;
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
        if (readLine[lineSize-1]== '\n'){
            readLine[lineSize-1]= '\0';
            lineSize--;
        }
        deliminator = findDeliminator(readLine);
        if(deliminator<0){
                code = INV_EVENT;
                free(readLine);
                break;
        }
        strncpy(lName,readLine,deliminator);
        lName[deliminator]= '\0';
       //checking for the end of the Event
        if (strcmp(lName,"END")== 0){
            exitFlag = true;
            if (strcmp(&(readLine[deliminator+1]),"VEVENT")==0){//might want tomake this safe
                if (createFlag&&startFlag&&uidFlag){
                    code = OK;
                }else{
                   code = INV_EVENT;
                }
            }else{
                code = INV_EVENT;
            }
        }else if(strcmp(lName,"UID")== 0){
            if (!uidFlag&&strcmp(&(readLine[deliminator+1]),"")!=0){
                strncpy((*useEvent)->UID,&(readLine[deliminator+1]),1000);
                uidFlag = true;
            }else{
               exitFlag = true;
                code = INV_EVENT;
            }
        }else if(strcmp(lName,"BEGIN")== 0){
             if (strcmp(&(readLine[deliminator+1]),"VALARM")==0){//might want tomake this safe
                tempAlarm = newAlarm();
                code = createAlarm(fp,&tempAlarm);
                if (code == OK){
                    insertBack((*useEvent)-> alarms,(void*)tempAlarm);
                    tempAlarm = NULL;
                }else{
                    deleteAlarm((void*)tempAlarm);
                    exitFlag = true;
                }
            }else{
               code = INV_EVENT;
            }
        }else if(strcmp(lName,"DTSTAMP")== 0){
            ammendedDT=strrchr(&(readLine[deliminator+1]),':');
            if(!(strstr(&(readLine[deliminator+1]),"TZID"))){ //this is for makeing special DT
                if (!createFlag){
                    if(ammendedDT){
                        code = createDateTime(&(ammendedDT[1]),&((*useEvent)->creationDateTime));
                    }else{
                        code = createDateTime(&(readLine[deliminator+1]),&((*useEvent)->creationDateTime));
                    }
                    if (code != OK){
                        exitFlag = true;
                    }else{
                        createFlag = true;
                    }
                }else{
                    exitFlag = true;
                    code = INV_EVENT;
                }
            }else{
                exitFlag = true;
                code = INV_DT;
            }
        }else if(strcmp(lName,"DTSTART")== 0){
            ammendedDT=strrchr(&(readLine[deliminator+1]),':');
            if(!(strstr(&(readLine[deliminator+1]),"TZID"))){
                if (!startFlag){
                    if(ammendedDT){
                        code = createDateTime(&(ammendedDT[1]),&((*useEvent)->startDateTime));
                    }else{
                        code = createDateTime(&(readLine[deliminator+1]),&((*useEvent)->startDateTime));
                    }                    
                    if (code != OK){
                        exitFlag = true;
                    }else{
                        startFlag = true;
                    }
                }else{
                    exitFlag = true;
                    code = INV_EVENT;
                }
            }else{
                exitFlag = true;
                code = INV_DT;
            }
        }else{
            if ((*useEvent)->properties&&!tempProp){
                code = createProperty(readLine,&tempProp);
                if (code == OK){
                    insertBack((*useEvent)->properties,(void*)tempProp);
                    tempProp = NULL;
                }else{
                    deleteProperty((void*)tempProp);
                    if(code == WRITE_ERROR){
                       code = INV_EVENT;
                    }
                    exitFlag = true;
                }
            }
        }
        free(readLine);
    }
    return code;

}

//returns a pointer to a newly alocated Alarm Struct
Alarm *newAlarm(){
    Alarm *newAlarm = NULL;

    newAlarm = malloc(sizeof(Alarm));
    if (newAlarm){
        newAlarm->trigger = NULL;

        newAlarm->properties = initializeList(&printProperty,&deleteProperty,&compareProperties);
    }else{
        printf("memory allocation request failed for new Alarm\n");
    }
    return newAlarm;
}
//Takes a file pointer and alarm stuct and assigns poperties acordingly
ICalErrorCode createAlarm(FILE *fp, Alarm **useAlarm){
    bool exitFlag= false;
    bool trigFlag = false;
    bool actionFlag = false;
    char *readLine = NULL;
    char lName[200];
    int lineSize = 0;
    int deliminator = -1;
    Property *tempProp = NULL;
    ICalErrorCode code= OTHER_ERROR;
    if (!(*useAlarm)){
        return INV_ALARM;
    }
    while (!exitFlag){
        lineSize = gline(&readLine,fp);
        if(lineSize<0){
            if(lineSize == -1){
                code = INV_ALARM;
                free(readLine);
                break;
            }else{
                code = OTHER_ERROR;
                free(readLine);
                break;
            }
        }
        //removing the new line characchter if it exists
        if (readLine[lineSize-1]== '\n'){
            readLine[lineSize-1]= '\0';
            lineSize--;
        }
        deliminator = findDeliminator(readLine);
        if(deliminator<0){
                code = INV_ALARM;
                free(readLine);
                break;
        }
        strncpy(lName,readLine,deliminator);
        lName[deliminator]= '\0';

        if (lineSize == -1){
            exitFlag = true;
            code = INV_ALARM;
            break;
        }
        //checking for the end of the alarm
        if (strcmp(lName,"END")== 0){
            exitFlag = true;
            if (strcmp(&(readLine[deliminator+1]),"VALARM")==0){//might want tomake this safe
                if (actionFlag&&trigFlag){
                    code = OK;
                }else{

                    code = INV_ALARM;
                }
            }else{
               code = INV_ALARM;
            }
        }else if(strcmp(lName,"ACTION")== 0){
            if (!actionFlag&&strcmp(&(readLine[deliminator+1]),"")!=0){
                strncpy(((*useAlarm)->action),&(readLine[deliminator+1]),200);
                actionFlag = true;
            }else{
                exitFlag = true;
               code = INV_ALARM;
            }
        }else if(strcmp(lName,"TRIGGER")== 0){
             if (!trigFlag&&strcmp(&(readLine[deliminator+1]),"")!=0){
                (*useAlarm)->trigger= malloc((lineSize-deliminator)*sizeof(char));
                if((*useAlarm)->trigger){
                    strcpy((*useAlarm)->trigger,&(readLine[deliminator+1]));
                    trigFlag = true;
                }
            }else{
               exitFlag = true;
               code = INV_ALARM;
                
            }           
        }else{
            if ((*useAlarm)->properties&&!tempProp){
                code = createProperty(readLine,&tempProp);
                if (code == OK){
                    insertBack((*useAlarm)->properties,(void*)tempProp);
                    tempProp = NULL;
                }else{
                    deleteProperty((void*)tempProp);
                    if(code == WRITE_ERROR){
                       code = INV_ALARM;
                    }
                    exitFlag = true;
                }
            }
        }
        free(readLine);
    }
    return code;
}

//returns a pointer to a newly alocated DateTime Struct
DateTime *newDateTime(){
    DateTime *newDT = NULL;
    newDT = malloc(sizeof(DateTime));
    if (newDT){

    newDT->UTC = false;
    }else{
        printf("memory allocation request failed for new DateTime\n");
    }
    return newDT;
}
//Takes a line with just date time component and event stuct and assigns poperties acordingly
ICalErrorCode createDateTime(char *inputLine, DateTime *useDateTime){
    char *token;
    char *holding;
    token = strtok(inputLine, "T");
    holding = token;
    token = strtok(NULL,"T");
    if(!token){
        return INV_DT;
    }
    if (strlen(holding)==8){
        holding[8]= '\0';
        strcpy(useDateTime->date,holding);
    }else{
        return INV_DT;
    }
    if (strlen(token)==7){
        if (token[6]=='Z'){
            useDateTime->UTC = true;
            strncpy(useDateTime->time,token,6);
            useDateTime->time[6]= '\0';
        }else{
           return INV_DT;
        }
    }else if(strlen(token)==6){
        token[6]= '\0';
        strcpy(useDateTime->time,token);
    }else{
        return INV_DT;
    }

    return OK;
}


//Takes a NULL file pointer and event stuct and assigns poperties acordingly
//handles the 
ICalErrorCode createProperty(char *inputLine, Property **useProperty){
    char tempPropName [200];
    int lineLen;
    int deliminator= -2;
    lineLen = strlen(inputLine);
    deliminator = findDeliminator(inputLine);
    
    if(deliminator<0){
        if(deliminator == -1){
            return WRITE_ERROR;//using write error as a place holder for no decription error
        }else{
            return OTHER_ERROR;
        }
    }
    strncpy(tempPropName,inputLine,deliminator);
    tempPropName[deliminator]= '\0';
    if(strcmp(tempPropName,"")==0||strcmp(&(inputLine[deliminator+1]),"")==0){
        return WRITE_ERROR;
    }
    Property *newProp = NULL;
    newProp = malloc(sizeof(Property)+((lineLen-deliminator)*sizeof(char)));
    if(newProp){
        strcpy(newProp->propName,tempPropName);
        strcpy(newProp->propDescr,&(inputLine[deliminator+1]));
    }else{
        return OTHER_ERROR;
    }
    *useProperty = newProp;
    return OK;
}

int findDeliminator(char *line){
    int lineLen;
    int deliminator;
    //char* lineHasColon;
    lineLen = strlen(line);
/*    lineHasColon = memchr(line,':',strlen(line));
    if(!lineHasColon){
        return -2;
    }
    */
    for (deliminator =0; deliminator<lineLen; deliminator++){
        if ((line[deliminator]==';'|| line[deliminator] == ':')&&line[deliminator-1]!='\\'){
            return deliminator;
        }
    }
    return -1;
}

//find the first oucurance of a given char in the string
//return the inddex in the array where that char occures
int findChar(char *str, char target){
    int strLen;
    int deliminator;
    //char* lineHasColon;
    strLen = strlen(str);
/*    lineHasColon = memchr(line,':',strlen(line));
    if(!lineHasColon){
        return -2;
    }
    */
    for (deliminator =0; deliminator<strLen; deliminator++){
        if (str[deliminator]== target){
            return deliminator;
        }
    }
    return -1;
}

//basicly mirroring the getline function from C++ for n = 0 
//returns the length of the line or -1 if EOF is reached and -2 if a memory request fails and -3 if 
//takes a file pinter and a empty char which represents 
int gline(char **dst,FILE *fp){
    char c = 'c';
    char *temp= NULL;
    char *fail;
    int length =0;
    while(c !=EOF && c!='\n'){
        length++;
        c = fgetc(fp);

        temp = realloc(temp,sizeof(char)*(length+1));
        if (temp){
            temp[length-1]= c;
            fail = temp;
        }else{
            if (fail){
                free(fail);
            }
            *dst = NULL;
            return -2;
        }
    }
    temp[length]='\0';
    *dst = temp;
    if (c !=EOF){
        temp[length-1]='\0';
        return length;
    }else{
        return -1;
    }
}

strNode *newStrNode(char *string){
    strNode *new;
    
    new = malloc(sizeof(strNode));
    new->item = malloc(strlen(string)+1);
    strcpy(new->item,string);
    new->next = NULL;
    return new;
}
void addStrNode(strNode ***list, strNode *toAdd){
    toAdd->next = **list;
    **list = toAdd;
}
void add2StrNode(strNode **list, strNode *toAdd){
    toAdd->next = *list;
    *list = toAdd;
}
void deleteNode(strNode *node){
    free(node->item);
    free(node);
}
void deleteList(strNode **list){
    strNode *temp;
    while(*list){
        temp = (*list)->next;
        deleteNode(*list);
        *list = temp;
    }
}

/*function to check weither or not an item is valid by comparing 
  it to a list of repeated values and a list of value
*/

bool isValidProperty(char *name,const char **validProps,strNode **list){
    bool occurOnce = false;
    bool rValue = false;
    strNode *temp = NULL;
    strNode *tList =NULL;
    tList = *list;
    while (tList){
        if (strcmp(name,tList->item)==0){
            return false;
        }
        tList = tList->next;
    }
    int i =0;
    while (validProps[i]){
        if (strcmp(name,validProps[i]) ==0){
            rValue = true;
            break;
        }
        if (strcmp(validProps[i],"AFTER THIS CAN ONLY OCCUR ONCE")==0){
            occurOnce = true;
        }
        i++;
    }
    if (rValue && occurOnce){
        /*if (strcmp(name,"DTEND")==0){
            temp = newStrNode("DURATION");
        }else if(strcmp(name,"DURATION")==0){
            temp = newStrNode("DTEND");
        }else{
            */
            temp = newStrNode((char *)validProps[i]);
        //}
        addStrNode(&list,temp);
        
    }
    return rValue;
}
/* Removes the first and last charachters from a string
   returns a copy of the string with the unwanted charachrers removed
*/

char *removeFirstLastCharsFromStr(char *originalStr){
    char* cleanStr;
    int orgLen;
    char temp[2];
    orgLen = strlen(originalStr);
    cleanStr = malloc(orgLen);
    if (!cleanStr){
        return NULL;
    }
    strcpy(cleanStr,"");
    int i;
    for (i =1;i<orgLen-1;i++){
        temp[0]= originalStr[i];
        temp[1]= '\0';
        strcat(cleanStr,temp);
    }
    strcat(cleanStr,"\0");
    return cleanStr;
}

