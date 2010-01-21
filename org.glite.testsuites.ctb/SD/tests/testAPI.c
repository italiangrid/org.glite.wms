/*
 *  Copyright (c) Members of the EGEE Collaboration. 2009.
 *  See http://public.eu-egee.org/partners/ for details on the
 *  copyright holders.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#include "ServiceDiscovery.h"

#include <glib.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


/* Name of the service to query */
char *name = NULL;

/* Site of the service to query */
char *site = NULL;

/* Type of the service to query */
char *type = NULL;

/* Host of the service to query */
char *host = NULL;

/* cli-specific command line options */
const char *options = "s:n:t:x";

/* Set if print_details should be shown */
int print_details = 0;

/* Set if only endpoint is needed */
int print_endpoint = 0;


/* Service Data list */
SDServiceDataList data_list;

static void error(const char *fmt, ...)
{
	char *p, *msg;
	va_list ap;

	va_start(ap, fmt);
	msg = g_strdup_vprintf(fmt, ap);
	va_end(ap);

	p = strchr(msg, ':');
	if (p)
	{
		int len = p - msg + 1;
		if (msg[len + 1] && !strncmp(msg, msg + len + 1, len))
			memmove(msg, msg + len + 1, strlen(msg) - len);
	}
	fprintf(stderr, "%s\n", msg);
	fflush(stderr);
	g_free(msg);
}

static void print_service_text(SDService *service) {
    if (print_endpoint)
    {
        printf("%s\n", service->endpoint);
    }
    else
    {
    	printf("\n");
		printf("Name: %s\n", service->name);
		printf("Type: %s\n", service->type);
	    printf("Endpoint: %s\n", service->endpoint);
	    printf("Version: %s\n", service->version);
    }
}

static void print_service_details_text(SDServiceDetails *serviceDetails){

	int i;
	SDServiceData *serviceData;

	printf("\n");
	printf("Name: %s\n", serviceDetails->name);
	printf("Type: %s\n", serviceDetails->type);
	printf("Endpoint: %s\n", serviceDetails->endpoint);
	printf("Version: %s\n", serviceDetails->version);
	printf("Site: %s\n", serviceDetails->site);
	if(serviceDetails->wsdl)printf("WSDL: %s\n", serviceDetails->wsdl);
	if(serviceDetails->administration)printf("Administrator: %s\n", serviceDetails->administration);

	if (serviceDetails->vos)
	{
		printf("VOs: ");
		for (i = 0; i < serviceDetails->vos->numNames-1; i++)
			printf("%s,", serviceDetails->vos->names[i]);
		printf("%s\n", serviceDetails->vos->names[serviceDetails->vos->numNames-1]);
		printf("\n");
	}

	if (serviceDetails->associatedServices)
	{
		printf("Associated Services\n");
		for(i=0; i < serviceDetails->associatedServices->numServices; i++)
		{
			printf("\t>\n");
			printf("\tName: %s\n", serviceDetails->associatedServices->services[i]->name);
			printf("\tType: %s\n", serviceDetails->associatedServices->services[i]->type);
			printf("\tEndpoint: %s\n", serviceDetails->associatedServices->services[i]->endpoint);
			printf("\tVersion: %s\n", serviceDetails->associatedServices->services[i]->version);
		}
	}

	if (serviceDetails->data && serviceDetails->data->numItems)
	{
		printf("Service Data:\n");
		for (i = 0; i < serviceDetails->data->numItems; i++)
		{
		    serviceData = &(serviceDetails->data->items[i]);
		    printf(" Key: %s - Value: %s\n", serviceData->key, serviceData->value);
		}
	}
}

static void print_service(SDService *service)
{
	SDException exc;
	SDServiceDetails *serviceDetails;

    if (print_details)
    {
        /* Need extra call for each service in the list to get details */
        serviceDetails = SD_getServiceDetails(service->name, &exc);
        if (exc.status != SDStatus_SUCCESS)
        {
            error(exc.reason);
            SD_freeServiceDetails(serviceDetails);
            SD_freeException(&exc);
            return;
        }
        else {
			print_service_details_text(serviceDetails);
        }
    }
    else {
		  print_service_text(service);
    }
}


static int print_service_list(SDServiceList *list){

	int i;

	if (!list)
		return 1;

	for (i = 0; i < list->numServices; i++)	{
        print_service(list->services[i]);
	}
	return 1;
}


static int runTest(int argc G_GNUC_UNUSED, char *argv[] G_GNUC_UNUSED) {

	SDServiceList *list;
	SDService *service;
	SDException exc;

	// if service name is specified using the -n flag
	if (name) {
		service = SD_getService(name, &exc);
		if(exc.status != SDStatus_SUCCESS) {
			error(exc.reason);
			SD_freeService(service);
			SD_freeException(&exc);
			return EXIT_FAILURE;
		} else {
			print_service(service);
		}
		// If host is specified
	} else if (host) {
		list = SD_listServicesByHost(type, host, NULL, &exc);
		if (exc.status != SDStatus_SUCCESS) {
			error(exc.reason);
			SD_freeServiceList(list);
			SD_freeException(&exc);
			return EXIT_FAILURE;
		}
		if (!print_service_list(list))
		return EXIT_FAILURE;
		SD_freeServiceList(list);
	}
	else
	{
		/** List the services ... */
		if (!data_list.numItems)
		{
			/* Without a data list */
			list = SD_listServices(type, site, NULL, &exc);
			if (exc.status != SDStatus_SUCCESS)
			{
				error(exc.reason);
				SD_freeServiceList(list);
				SD_freeException(&exc);
				return EXIT_FAILURE;
			}
			if (!print_service_list(list))
			return EXIT_FAILURE;
			SD_freeServiceList(list);
		}
		else
		{
			list = SD_listServicesByData(&data_list, type, site, NULL, &exc);
			if (exc.status != SDStatus_SUCCESS)
			{
				error(exc.reason);
				if (list) SD_freeServiceList(list);
				SD_freeException(&exc);
				return EXIT_FAILURE;
			}
			if (!print_service_list(list))
			return EXIT_FAILURE;
			SD_freeServiceList(list);
		}

	}

	return EXIT_SUCCESS;
}


int main(int argc, char* argv[]){

	int exitCode, c;

	while ((c = getopt(argc, argv, options)) != -1)

		switch (c)	{
		// -n : Service name flag
		case 'n':
			name = g_strdup(optarg);
			break;
		// -t : Service type flag
		case 't':
			type = g_strdup(optarg);
			break;
		// -s : Grid Site flag
		case 's':
			site = g_strdup(optarg);
			break;
		// -x : Print service details flag
		case 'x':
			print_details = 1;
			print_endpoint = 0;
			break;
		case ':':
			error("Option argument is missing\n");
			exit(EXIT_FAILURE);
		case '?':
			error("Unknown command line option\n");
			exit(EXIT_FAILURE);
		default:
            error("Illegal command line arguments\n");
            exit(EXIT_FAILURE);
		}

	exitCode = runTest(argc, argv);

	return exitCode;

}
