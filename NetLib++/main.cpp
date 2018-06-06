#include <phpcpp.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
extern "C" {

    
    
class PublicClass : public Php::Base
{
public:
        /**
         *  C++ constructor and destructor
         */
        PublicClass() {}
        virtual ~PublicClass() {}
    
        struct MemoryStruct {
            char *memory;
            size_t size;
        };
    
static size_t
        WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
        {
            size_t realsize = size * nmemb;
            struct MemoryStruct *mem = (struct MemoryStruct *)userp;
            
            mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
            if(mem->memory == NULL) {
                /* out of memory! */
                printf("not enough memory (realloc returned NULL)\n");
                return 0;
            }
            
            memcpy(&(mem->memory[mem->size]), contents, realsize);
            mem->size += realsize;
            mem->memory[mem->size] = 0;
            
            return realsize;
        }
        
        /**
         *  Function that is to get web page src
         *
         *  @params Php::Parameters c vector type(a web url)
         *
         *  @return Php::Value   web src
         */
static  Php::Value
    get(Php::Parameters &params)
        {
            /* return value */
            Php::Value ret="";
            
            curl_global_init(CURL_GLOBAL_ALL);/* init curl */
            
            CURL *curl_handle = curl_easy_init(); /* init curl object */
            
            if(curl_handle) {
                
                CURLcode res; /* defined curl return res */
                
                std::string str=params[0];
                
                const char *str_c = str.c_str();
                
                /* setting the request header */
                curl_easy_setopt(curl_handle, CURLOPT_URL,str_c);
                
                struct MemoryStruct chunk;/* defined structure body */
                
                chunk.memory = (char *)malloc(1);  /* will be grown as needed by the realloc above */
                
                chunk.size = 0;    /* no data at this point , init to zero */
                
                /* send all data to this function  */
                curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
                
                /* we pass our 'chunk' struct to the callback function */
                curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
                
                /* some servers don't like requests that are made without a user-agent
                 field, so we provide one */
                curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
                
                /* get it! send request*/
                res = curl_easy_perform(curl_handle);
                
                /* check for errors */
                if(res != CURLE_OK) {
                    /* error,print error message */
                    //Php::error << curl_easy_strerror(res) << std::flush;
                    return ret;
                }
                else {
                    /*
                     * Now, our chunk.memory points to a memory block that is chunk.size
                     * bytes big and contains the remote file.
                     *
                     * Do something nice with it!
                     */
                    ret=chunk.memory;
                }
                
                /* cleanup curl stuff */
                curl_easy_cleanup(curl_handle);
                
                free(chunk.memory);
                
                /* we're done with libcurl, so clean it up */
                curl_global_cleanup();
                return ret; /* clean all and return */
            }
        }
    };


    
    /**
     *  Function that is called by PHP right after the PHP process
     *  has started, and that returns an address of an internal PHP
     *  strucure with all the details and features of your extension
     *
     *  @return void*   a pointer to an address that is understood by PHP
     */
    PHPCPP_EXPORT void *get_module() 
    {
        // static(!) Php::Extension object that should stay in memory
        // for the entire duration of the process (that's why it's static)
        static Php::Extension extension("netlib++", "1.0");
      
        // @todo    add your own functions, classes, namespaces to the extension
        
        /* create public class */
        Php::Class<PublicClass> netLib("NetLib");
        netLib.method("get", &PublicClass::get);
        
        // add the class to the extension
        extension.add(std::move(netLib));
        
        // return the extension
        return extension;
        
    }
}
