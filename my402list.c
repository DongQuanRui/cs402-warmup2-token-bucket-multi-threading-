#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "my402list.h"

int My402ListLength(My402List* list){
    if((list->anchor).next == &(list->anchor) && (list->anchor).prev == &(list->anchor)) {
        return 0;
    }else{
        My402ListElem *temp;
        int length = 0;
        for (temp = &(list)->anchor; temp->next != &(list)->anchor; temp = temp->next) {
            length++;
        }
        return length;
    }
}

int My402ListEmpty(My402List* list){
    if((list->anchor).next == &(list->anchor)) {
        return TRUE;
    }else{
        return FALSE;
    }
}

int My402ListAppend(My402List *list, void *obj){
    if (My402ListEmpty(list)) {
        My402ListElem *ele = (My402ListElem *)malloc(sizeof(My402ListElem));
        (list->anchor).next = ele;
        (list->anchor).prev = ele;
        ele->next = &(list->anchor);
        ele->prev = &(list->anchor);
        ele->obj = obj;
    }
    else{
        My402ListElem *ele = (My402ListElem *)malloc(sizeof(My402ListElem));
        ele->next = &(list->anchor);
        ele->prev = (list->anchor).prev;
        (list->anchor).prev->next = ele;
        (list->anchor).prev = ele;
        ele->obj = obj;
    }
    return 0;
}

int My402ListPrepend(My402List* list, void* obj){
    if (My402ListEmpty(list)) {
        My402ListElem *ele = (My402ListElem *)malloc(sizeof(My402ListElem));
        (list->anchor).next = ele;
        (list->anchor).prev = ele;
        ele->next = &(list->anchor);
        ele->prev = &(list->anchor);
        ele->obj = obj;
    }
    else{
        My402ListElem *ele = (My402ListElem *)malloc(sizeof(My402ListElem));
        ele->prev = &(list->anchor);
        ele->next = (list->anchor).next;
        (list->anchor).next->prev = ele;
        (list->anchor).next = ele;
        ele->obj = obj;
    }
    return 0;
}

void My402ListUnlink(My402List* list, My402ListElem *elem){
    if(!My402ListEmpty(list)){
        (elem->next)->prev = (elem->prev);
        (elem->prev)->next = (elem->next);
        free(elem);
    }
}

void My402ListUnlinkAll(My402List *list){
    if (!My402ListEmpty(list)) {
        while ((list->anchor).next != &(list->anchor)) {
            My402ListElem *temp = (list->anchor).next;
            My402ListUnlink(list, temp);
        }
    }
}

int My402ListInsertAfter(My402List *list, void *obj, My402ListElem *elem){
    if (My402ListEmpty(list)) {
        My402ListAppend(list, obj);
    }else{
        My402ListElem *ele = (My402ListElem *)malloc(sizeof(My402ListElem));
        ele->next = elem->next;
        ele->prev = elem;
        elem->next->prev = ele;
        elem->next = ele;
        ele->obj = obj;
    }
    return 0;
}

int My402ListInsertBefore(My402List *list, void *obj, My402ListElem *elem){
    if (My402ListEmpty(list)) {
        My402ListPrepend(list, obj);
    }else{
        My402ListElem *ele = (My402ListElem *)malloc(sizeof(My402ListElem));
        ele->prev = elem->prev;
        ele->next = elem;
        elem->prev->next = ele;
        elem->prev = ele;
        ele->obj = obj;
    }
    return 0;
}

My402ListElem *My402ListFirst(My402List *list){
    if (My402ListEmpty(list)) {
        return NULL;
    }
    else{
        My402ListElem *temp = (list->anchor).next;
        return temp;
    }
}

My402ListElem *My402ListLast(My402List *list){
    if (My402ListEmpty(list)) {
        return NULL;
    }
    else{
        My402ListElem *temp = (list->anchor).prev;
        return temp;
    }
}

My402ListElem *My402ListNext(My402List *list, My402ListElem *elem){
    if (elem->next != &(list->anchor)) {
        return elem->next;
    }else{
        return NULL;
    }
}

My402ListElem *My402ListPrev(My402List *list, My402ListElem *elem){
    if (elem->prev != &(list->anchor)) {
        return elem->prev;
    }else{
        return NULL;
    }
}

My402ListElem* My402ListFind(My402List *list, void *obj){
    if (My402ListEmpty(list)) {
        return NULL;
    }else{
        My402ListElem *temp = NULL;
        for (temp = &(list->anchor); temp->next != &(list->anchor); temp = temp->next) {
            if (temp->obj == obj) {
                temp = temp;
                break;
            }
        }
        if (temp->obj != obj) {
            return NULL;
        }else{
            return temp;
        }
    }
}

void My402ListTraverse(My402List *list){
    My402ListElem *temp = NULL;
    for (temp = &(list->anchor); temp->next != &(list)->anchor; temp = temp->next) {
        printf("%4d", (int)(temp->next)->obj);
    }
    printf("\n");
}

int My402ListInit(My402List *list){
    if ((list->anchor).next == NULL && (list->anchor).prev == NULL) {
        (list->anchor).obj = NULL;
        (list->anchor).next = &(list->anchor);
        (list->anchor).prev = &(list->anchor);
    }
    return 0;
}
