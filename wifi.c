#include <stdio.h>
#include  <stdio.h>
#include "esp_wifi.h"
#include "lwip/ip4_addr.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_eap_client.h"


#define PLACE_SCHOOL
//#define PLACE_HOME 

#if defined (PLACE_SCHOOL)
char *ssid = "";
char *wifi_user = "";
char *eap_id = (char *) "eap_id" ;
char *password = "";
#elif defined (PLACE_HOME)
char *ssid = ""; 
char *password = ""; 
#endif

static void event_handler(void* arg, esp_event_base_t ebase, int32_t eid, void* edata)
{
    if (ebase == WIFI_EVENT && eid == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (ebase == WIFI_EVENT && eid == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
    } else if (ebase == IP_EVENT && eid == IP_EVENT_STA_GOT_IP ) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) edata;
        char localip[128] = {} ;
        strcpy(localip, ip4addr_ntoa( (struct ip4_addr *) &event->ip_info.ip) );
        printf("localip:%s", localip);
    }
}


void wifi_main(void)
{
    //LOAD NETIF and SETUP EVENTLOOP
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);


    //CONFIG AND START
    wifi_config_t wifi_config = {};
    strcpy( (char *) wifi_config.sta.ssid, ssid);
    strcpy( (char *) wifi_config.sta.password, password);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK ;
    wifi_config.sta.pmf_cfg.capable = true ;
    wifi_config.sta.pmf_cfg.required = false ;
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

#if defined (PLACE_SCHOOL) 
    esp_wifi_disable_pmf_config(WIFI_IF_STA);
    esp_eap_client_set_username((uint8_t *)wifi_user, strlen(wifi_user));
    esp_eap_client_set_password((uint8_t *)password, strlen(password));
    esp_eap_client_set_identity((uint8_t *)eap_id, strlen(eap_id));
    esp_wifi_sta_enterprise_enable();
#endif

    esp_wifi_start();

    printf("wifi done\n");
}
