#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

void performRequest(const char* url) {
    // https://curl.se/libcurl/c/http-post.html
    CURL* curl;
    CURLcode res;

    /* get CURL handle */
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error initializing CURL\n");
        return;
    }

    /* Set the request URL */
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* Perform the request */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return;
    }

    // Cleanup
    curl_easy_cleanup(curl);
}

int main() {
    char userInput[10];

    while(1){
        /* get user input */
        printf("\nEnter 'increment' or 'imprimir': ");
        scanf("%s", userInput);

        /* request URL according to user input */
        char requestURL[50];
        if (strcmp(userInput, "increment") == 0) {
            system("curl -X POST http://localhost:8537/increment");
        } 
        else if (strcmp(userInput, "imprimir") == 0) {
            snprintf(requestURL, sizeof(requestURL), "http://localhost:8537/imprimir");
            /* Do the request */
            performRequest(requestURL);
        }
        else {
            printf("Client disconnected\n");
            exit(EXIT_SUCCESS);
        }
        
    }

    return 0;
}
