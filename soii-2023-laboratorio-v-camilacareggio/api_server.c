#include <stdio.h>
#include <ulfius.h>

int counter = 0;

/* Endpoint: increment counter */
int increment_endpoint(const struct _u_request *request, struct _u_response *response, void *user_data) {
    counter++;
    ulfius_set_string_body_response(response, 200, "Counter incremented");
    return U_CALLBACK_CONTINUE;
}

/* Endpoint: get counter value */
int imprimir_endpoint(const struct _u_request *request, struct _u_response *response, void *user_data) {
    char buffer[50];
    snprintf(buffer, sizeof(buffer), "Counter value: %d", counter);
    ulfius_set_string_body_response(response, 200, buffer);
    return U_CALLBACK_CONTINUE;
}

int main(int argc, char **argv) {
    struct _u_instance instance;

    /* Init Ulfius */
    if (ulfius_init_instance(&instance, 8537, NULL, NULL) != U_OK) {
        fprintf(stderr, "Error initializing Ulfius\n");
        return (1);
    }

    /* Add endpoints */
    ulfius_add_endpoint_by_val(&instance, "POST", "/increment", NULL, 0, increment_endpoint, NULL);
    ulfius_add_endpoint_by_val(&instance, "GET", "/imprimir", NULL, 0, imprimir_endpoint, NULL);

    /* Init server */
    if (ulfius_start_framework(&instance) == U_OK) {
        printf("Server on port %d\n", instance.port);

        /* server stops on enter */
        getchar();
        ulfius_stop_framework(&instance);
        printf("Server off\n");
    } else {
        fprintf(stderr, "Error starting server\n");
    }

    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);

    return 0;
}