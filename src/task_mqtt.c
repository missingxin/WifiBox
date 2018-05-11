#include "tasks.h"
#include "../lib/MQTTClient/include/MQTTClient.h"
#include "defaultSettings.h"

#define MQTT_SERVER_PORT 1883
#define MQTT_SERVER_URL DEFAULT_MQTT_BROKER_URL

void messageArrived(MessageData* data){
int i =0;
    xTaskCreate(LEDBlinkTask, (signed char *)"Blink", 256, NULL, 2, NULL);
    printf("Message arrived on topic:%s \n",data->message->payload);
    printf("Size of data %d\n",(int)data->message->payloadlen );
    for(i=0; i<(int)data->message->payloadlen;i++)
        printf("%c",((char *)data->message->payload)[i]);
}

void prvMQTTEchoTask(void *pvParameters){
    /* connect to m2m.eclipse.org, subscribe to a topic, send and receive messages regularly every 1 sec */
    MQTTClient client;
    Network network;
    unsigned char sendbuf[80], readbuf[80];
    int rc = 0, count = 0;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    pvParameters = 0;
    NetworkInit(&network);
    MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

    char* address = MQTT_SERVER_URL;
    if ((rc = ConnectNetwork(&network, address, MQTT_SERVER_PORT )) != 0)
        printf("Return code from network connect is %d\n", rc);

#if defined(MQTT_TASK)
    if ((rc = MQTTStartTask(&client)) != pdPASS)
        printf("Return code from start tasks is %d\n", rc);
#endif

    connectData.MQTTVersion = 3;
    connectData.clientID.cstring = "FreeRTOS_sample";

    if ((rc = MQTTConnect(&client, &connectData)) != 0)
        printf("Return code from MQTT connect is %d\n", rc);
    else
        printf("MQTT Connected\n");

    if ((rc = MQTTSubscribe(&client, "/+/+/+/+/+/+", 2, messageArrived)) != 0)
        printf("Return code from MQTT subscribe is %d\n", rc);

    while (++count)
    {
        MQTTMessage message;
        char payload[30];

        message.qos = 1;
        message.retained = 0;
        message.payload = payload;
        sprintf(payload, "message number %d\n", count);
        message.payloadlen = strlen(payload);

        if ((rc = MQTTPublish(&client, "/SPjoseph/seniorBOX/000/server/test/0", &message)) != 0)
            printf("Return code from MQTT publish is %d\n", rc);
#if !defined(MQTT_TASK)
        if ((rc = MQTTYield(&client, 10000)) != 0)
            printf("Return code from yield is %d\n", rc);
#endif
    }
}

