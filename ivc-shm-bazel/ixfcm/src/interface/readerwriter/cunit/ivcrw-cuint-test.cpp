#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdio.h>
#include "IvcReaderWriter.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>

bool readCallBack(readerObj &obj) {
    return 0;
}

bool writeCallBack(writerObj &obj) {
    return 0;
}

void setInterfaceConfig_test() {

    cIvcReaderWriter *obj = new cIvcReaderWriter();
    int32_t retval;

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_READER, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_READER, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_READER, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_READER, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_SYNC_READER, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_SYNC_READER, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_SYNC_READER, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_SYNC_READER, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_SYNC_READER, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_SYNC_READER, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER, "topic", 10, &readCallBack, &writeCallBack, NULL);
    //printf("return value = %d\n", retval);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_ASYNC_READER, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_ASYNC_READER, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_ASYNC_READER, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_ASYNC_READER, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_SYNC_READER, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_SYNC_READER, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_SYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, &writeCallBack, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "", 10, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 0, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", -1, NULL, NULL, NULL);
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    delete obj;
}

void startInterface_test()
{
    cIvcReaderWriter *obj = new cIvcReaderWriter();
    int32_t retval;

    retval = obj->startInterface();
    CU_ASSERT_EQUAL(retval,1);

    retval = obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "", 10, NULL, NULL, NULL);
    retval = obj->startInterface();
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    retval = obj->startInterface();
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    retval = obj->startInterface();
    obj->stopInterface();
    obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    retval = obj->startInterface();
    CU_ASSERT_EQUAL(retval,0);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", -10, NULL, NULL, NULL);
    retval = obj->startInterface();
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 0, NULL, NULL, NULL);
    retval = obj->startInterface();
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    retval = obj->startInterface();
    retval = obj->startInterface();
    CU_ASSERT_EQUAL(retval,1);
    obj->stopInterface();

    delete obj;
}

void pushData_test()
{
    cIvcReaderWriter *obj = new cIvcReaderWriter();
    int32_t retval;
    std::string data = "ganesh";
    const uint8_t *message = reinterpret_cast<const uint8_t*>(&data[0]);

    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 1);
    obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 1);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_READER, "topic", 10, NULL, NULL, NULL);
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 1);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    obj->startInterface();
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 0);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 4, NULL, NULL, NULL);
    obj->startInterface();
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 1);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_READER, "topic", 10, NULL, NULL, NULL);
    obj->startInterface();
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 1);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_READER | LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    obj->startInterface();
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 0);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    obj->startInterface();
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 1);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    obj->startInterface();
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 1);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    obj->startInterface();
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 1);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_ASYNC_WRITER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    obj->startInterface();
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 0);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    obj->startInterface();
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 1);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_WRITER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    obj->startInterface();
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 1);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    obj->startInterface();
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 1);
    obj->stopInterface();

    obj->setInterfaceConfig(LGIVC_SYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, NULL, &writeCallBack, NULL);
    obj->startInterface();
    retval = obj->pushData(message, data.length());
    CU_ASSERT_EQUAL(retval, 1);
    obj->stopInterface();

    delete obj;
}

void pullData_test()
{
    cIvcReaderWriter *wobj = new cIvcReaderWriter();
    cIvcReaderWriter *robj = new cIvcReaderWriter();
    int32_t retval;
    uint8_t *message = NULL;

    retval = robj->pullData(message, 7);
    CU_ASSERT_EQUAL(retval, 1);

    message = (uint8_t*)malloc(10);
    retval = robj->pullData(message, 0);
    CU_ASSERT_EQUAL(retval, 1);

    retval = robj->pullData(message, 7);
    CU_ASSERT_EQUAL(retval, 1);

    wobj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    retval = wobj->pullData(message, 6);
    CU_ASSERT_EQUAL(retval, 1);

    wobj->setInterfaceConfig(LGIVC_ASYNC_WRITER, "topic", 10, NULL, &writeCallBack, NULL);
    retval = wobj->pullData(message, 6);
    CU_ASSERT_EQUAL(retval, 1);

    robj->setInterfaceConfig(LGIVC_SYNC_READER, "topic", 10, NULL, NULL, NULL);
    retval = robj->pullData(message, 6);
    CU_ASSERT_EQUAL(retval, 1);

    robj->setInterfaceConfig(LGIVC_SYNC_READER, "topic", 10, NULL, NULL, NULL);
    robj->startInterface();
    retval = robj->pullData(message, 6);
    CU_ASSERT_EQUAL(retval, 1);
    robj->stopInterface();

    std::string data = "ganesh2";
    const uint8_t *smessage = reinterpret_cast<const uint8_t*>(&data[0]);
    wobj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    wobj->startInterface();

    robj->setInterfaceConfig(LGIVC_SYNC_READER, "topic", 10, NULL, NULL, NULL);
    robj->startInterface();
    wobj->pushData(smessage, data.length());
    retval = robj->pullData(message, 7);
    CU_ASSERT_EQUAL(retval, 0);
    retval = robj->pullData(message, 7);
    CU_ASSERT_EQUAL(retval, 0);

    wobj->pushData(smessage, data.length());
    retval = robj->pullData(message, 12);
    CU_ASSERT_EQUAL(retval, 1);
    robj->stopInterface();

    robj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    robj->startInterface();
    retval = wobj->pushData(smessage, data.length());
    retval = robj->pullData(message, 7);
    CU_ASSERT_EQUAL(retval, 0);
    robj->stopInterface();

    robj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    robj->startInterface();
    retval = wobj->pushData(smessage, data.length());
    retval = robj->pullData(message, 7);
    CU_ASSERT_EQUAL(retval, 1);

    wobj->stopInterface();
    robj->stopInterface();

    free(message);
    delete wobj;
    delete robj;
}

void read_test()
{
    cIvcReaderWriter *wobj = new cIvcReaderWriter();
    cIvcReaderWriter *robj = new cIvcReaderWriter();
    int32_t retval;
    uint8_t *message = NULL;

    retval = robj->read(message, 7);
    CU_ASSERT_EQUAL(retval, 1);

    message = (uint8_t*)malloc(10);
    retval = robj->read(message, 0);
    CU_ASSERT_EQUAL(retval, 1);

    retval = robj->read(message, 7);
    CU_ASSERT_EQUAL(retval, 1);

    wobj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    retval = wobj->read(message, 6);
    CU_ASSERT_EQUAL(retval, 1);

    wobj->setInterfaceConfig(LGIVC_ASYNC_WRITER, "topic", 10, NULL, &writeCallBack, NULL);
    retval = wobj->read(message, 6);
    CU_ASSERT_EQUAL(retval, 1);

    robj->setInterfaceConfig(LGIVC_SYNC_READER, "topic", 10, NULL, NULL, NULL);
    retval = robj->read(message, 6);
    CU_ASSERT_EQUAL(retval, 1);

    robj->setInterfaceConfig(LGIVC_SYNC_READER, "topic", 10, NULL, NULL, NULL);
    robj->startInterface();
    retval = robj->read(message, 6);
    CU_ASSERT_EQUAL(retval, 1);
    robj->stopInterface();

    std::string data = "ganesh2";
    const uint8_t *smessage = reinterpret_cast<const uint8_t*>(&data[0]);
    wobj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    wobj->startInterface();

    robj->setInterfaceConfig(LGIVC_SYNC_READER, "topic", 10, NULL, NULL, NULL);
    robj->startInterface();
    wobj->pushData(smessage, data.length());
    retval = robj->read(message, 7);
    CU_ASSERT_EQUAL(retval, 0);
    retval = robj->read(message, 7);
    CU_ASSERT_EQUAL(retval, 0);

    wobj->pushData(smessage, data.length());
    retval = robj->read(message, 12);
    CU_ASSERT_EQUAL(retval, 1);
    robj->stopInterface();

    robj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_EVENTCALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    robj->startInterface();
    retval = wobj->pushData(smessage, data.length());
    retval = robj->read(message, 7);
    CU_ASSERT_EQUAL(retval, 0);
    robj->stopInterface();

    robj->setInterfaceConfig(LGIVC_ASYNC_READER | LGIVC_WITH_DATACALLBACK, "topic", 10, &readCallBack, NULL, NULL);
    robj->startInterface();
    retval = wobj->pushData(smessage, data.length());
    retval = robj->read(message, 7);
    CU_ASSERT_EQUAL(retval, 1);

    wobj->stopInterface();
    robj->stopInterface();

    free(message);
    delete wobj;
    delete robj;
}

void stopInterface_test()
{
    cIvcReaderWriter *obj = new cIvcReaderWriter();
    cIvcReaderWriter *obj1 = new cIvcReaderWriter();
    int32_t retval;

    retval = obj->stopInterface();
    CU_ASSERT_EQUAL(retval,1);

    obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    retval = obj->stopInterface();
    CU_ASSERT_EQUAL(retval,1);

    obj->setInterfaceConfig(LGIVC_SYNC_WRITER, "topic", 10, NULL, NULL, NULL);
    obj->startInterface();
    retval = obj->stopInterface();
    CU_ASSERT_EQUAL(retval,0);
    retval = obj->stopInterface();
    CU_ASSERT_EQUAL(retval,1);

    delete obj;
    delete obj1;
}

int main() {

    CU_initialize_registry();

    CU_pSuite suite = CU_add_suite("suite1", 0, 0);

    CU_add_test(suite, "setInterfaceConfig_test", setInterfaceConfig_test);

    CU_add_test(suite, "startInterface_test", startInterface_test);

    CU_add_test(suite, "pushData_test", pushData_test);

    CU_add_test(suite, "pullData_test", pullData_test);

    CU_add_test(suite, "read_test", read_test);

    CU_add_test(suite, "stopInterface_test", stopInterface_test);

    CU_basic_set_mode(CU_BRM_VERBOSE);

    CU_basic_run_tests();

    CU_cleanup_registry();

    printf("exit main\n");
    return 0;
}
