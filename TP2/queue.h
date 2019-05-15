#pragma once 

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#include "sope.h"
#include "variables.h"

/**
 * ----- Queue funtions ------
 **/
void inicializeRequests(){
        queue.first = 0;
        queue.last = -1;
        queue.itemCount = 0;
}

bool isEmpty() {
        return queue.itemCount == 0;
}

bool isFull() {
        return queue.itemCount == MAX_REQUESTS;
}

void insert(tlv_request_t request) {

        if(!isFull()) {

                if(queue.last == MAX_REQUESTS-1) {
                        queue.last = -1;
                }

                queue.requestArray[++queue.last] = request;
                queue.itemCount++;
        }
}

tlv_request_t removeRequest() {
        tlv_request_t request = queue.requestArray[queue.first++];

        if(queue.first == MAX_REQUESTS) {
                queue.first = 0;
        }

        queue.itemCount--;
        return request;
}
/**
 * ----------------------------
 **/