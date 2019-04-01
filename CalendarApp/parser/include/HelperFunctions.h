/*  HelperFunction.h
    Header File for Helper Functions:.
    By: James Arthur Anderson
    Student ID #: 1013076
    Date: January. 28, 2019
*/
/*including of standard libraries*/
#include<stdio.h>
#include"CalendarParser.h"
#include"LinkedListAPI.h"

#ifndef HELPER_H
#define HELPER_H

//Takes the name of a File and a empty file pointer and makes a new file wich has all of its lines unfolded
ICalErrorCode lineDefold(char *orignalFileName, FILE *unfoldedFile);

//returns a pointer to a newly alocated Event Struct
Event *newEvent();
//Takes a file pointer and event stuct and assigns poperties acordingly
ICalErrorCode createEvent(FILE *fp, Event **useEvent);

//returns a pointer to a newly alocated Alarm Struct
Alarm *newAlarm();
//Takes a file pointer and alarm stuct and assigns poperties acordingly
ICalErrorCode createAlarm(FILE *fp, Alarm **useAlarm);

//returns a pointer to a newly alocated DateTime Struct
DateTime *newDateTime();
//Takes a file pointer and event stuct and assigns poperties acordingly
ICalErrorCode createDateTime(char *inputLine, DateTime *useDateTime);


//Takes a file line and null property stuct and assigns poperties acordingly
ICalErrorCode createProperty(char *inputLine, Property **useProperty);

int findDeliminator(char *line);

//find the first oucurance of a given char in the string
//return the inddex in the array where that char occures
int findChar(char *str, char target);

int gline(char **dst,FILE *fp);

//string linked list function
typedef struct stringNodeStruct strNode;
struct stringNodeStruct{
    char *item;
    strNode *next;
};
strNode *newStrNode(char *string);
void addStrNode(strNode ***list, strNode *toAdd);
void add2StrNode(strNode **list, strNode *toAdd);
void deleteNode(strNode *node);
void deleteList(strNode **list);

/*function to check weither or not an item is valid by comparing 
  it to a list of repeated values and a list of value
*/

bool isValidProperty(char *name,const char **validProps,strNode **list);
/* Removes the first and last charachters from a string
   returns a copy of the string with the unwanted charachrers removed
*/
char *removeFirstLastCharsFromStr(char *originalStr);

#endif


  
