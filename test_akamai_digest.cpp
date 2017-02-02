// To build: g++  -g -lssl -o test_akamai_digest test_akamai_digest.cpp


#include <string>
#include <iomanip>
#include <iostream>
#include <openssl/evp.h>
#include <time.h>
#include <malloc.h>

#define URL_TOKEN_LEN 32        /* Length of output token not including terminator */

#define BIN2HEX(c,h,l) ((h) = (c) / 16, (l) = (c) % 16, \
                        (h) = ((h) > 9 ? 'a' + (h) - 10 : '0' + (h)),\
                        (l) = ((l) > 9 ? 'a' + (l) - 10 : '0' + (l)))

/**************************************************************************
 * Converts contents of *in to hex and places the output in *out.
 * Caller is responsable for allocating enough memory for *out.
 **************************************************************************/
static unsigned int akamaiBin2Hex(unsigned char *in, unsigned int in_len, unsigned char *out)
{
    unsigned int  in_pos, out_pos;
    unsigned char h, l;

    for (in_pos = 0, out_pos = 0; in_pos < in_len; in_pos++)
    {
        BIN2HEX(in[in_pos], h, l);
        out[out_pos++] = h;
        out[out_pos++] = l;
    }

    out[out_pos] = 0;
    return out_pos;
}


/*****************************************************************************
 * Generates the authentication token.
 *  int urlauth_gen_token(
 *      const char *url,          in: URL
 *      time_t      start_time,   in: Start time for token, 0 for current time
 *      time_t      window,       in: valid window in seconds 
 *      const char *salt,         in: salt from metadata
 *      const char *extract,      in: extracted value (cookie, etc)
 *      time_t     *expires,      out: expiration time
 *      char       **token)       out: generated token
 *
 * 
 * If expires and token parameters are nul memory will be allocated
 * with malloc to hold the results.
 *
 * If token is non-null, it MUST be at least 33 bytes long.
 *
 * will return  < 0 (and not allocate any memory) if url or salt are null or zero 
 * length.
 * will return < 0 if malloc for expires or token fails oir if any of the 
 * WIN32 MD5 functions fail. 
 *****************************************************************************/
int urlauth_gen_token(const char *url, time_t start_time, time_t window, 
                      const char *salt, const char *extract, time_t *expires,
                      char **token)
{
    EVP_MD_CTX    md_ctx;                   /* message digest context (OpenSSL struct) */
    unsigned char md_buf[EVP_MAX_MD_SIZE];  /* message digest buf */

    char *output;
    unsigned char expires_buf[4];           /* buffet for exp time, for portability */
    unsigned int           md_buf_len   = 0;         /* # of bytes in md_buf */
    int           malloc_token = 0;         /* did we malloc a token buf? */
    int           malloc_exp   = 0;         /* did we malloc an expires buf? */
    unsigned int  token_len = 0;            /* # of bytes in token[] */
    unsigned int  url_len      = 0;
    unsigned int  salt_len     = 0;
    unsigned int  extract_len = (extract ? strlen(extract) : 0);

    if (url == NULL)
        return -1;
    url_len = strlen(url);
    if (url_len == 0)
        return -2;
    if (salt == NULL)
        return -3;
    salt_len = strlen(salt);
    if (salt_len == 0)
        return -4;

    if (start_time == 0) {
        start_time = time(NULL);
    }
    if (expires == NULL) {
        expires = (time_t *)malloc(sizeof(time_t));
        if (expires == NULL)
            return -5;
        malloc_exp = 1;
    }
    if (*token == NULL) {
        output = (char *)malloc(URL_TOKEN_LEN + 1);
        if (output == NULL) {
            if (malloc_exp)
                free(expires);
            return -6;
        }
        malloc_token = 1;
    }
        /* the buffer better be long enough! */
	*token = output;
    memset(output, '\0', URL_TOKEN_LEN + 1);

    *expires = start_time + window;

        /* we do this solely to ensure the correct byte order for */
        /* all platforms. */
    expires_buf[0] = (unsigned char) ((*expires) & 0xff);
    expires_buf[1] = (unsigned char) (((*expires) >> 8)  & 0xff);
    expires_buf[2] = (unsigned char) (((*expires) >> 16)  & 0xff);
    expires_buf[3] = (unsigned char) (((*expires) >> 24)  & 0xff);
        

    std::cout << "expires=" << *expires <<  "-0x" << std::hex << *expires << std::endl;
    std::cout << "expires_buf[]="
              << static_cast<unsigned>(expires_buf[0]) << ":" 
              << static_cast<unsigned>(expires_buf[1]) << ":"
              << static_cast<unsigned>(expires_buf[2]) << ":"
              << static_cast<unsigned>(expires_buf[3]) << std::endl;

        /* Calculate the first digest */
    EVP_DigestInit(&md_ctx, EVP_md5());
    EVP_DigestUpdate(&md_ctx, expires_buf, sizeof(expires_buf));
    EVP_DigestUpdate(&md_ctx, url, url_len);
    if (extract) {
        EVP_DigestUpdate(&md_ctx, extract, extract_len);
    }
    EVP_DigestUpdate(&md_ctx, salt, salt_len);
    EVP_DigestFinal(&md_ctx, md_buf, &md_buf_len);

        /* Calculate the second digest based on the first */
    EVP_DigestInit(&md_ctx, EVP_md5());
    EVP_DigestUpdate(&md_ctx, salt, salt_len);
    EVP_DigestUpdate(&md_ctx, md_buf, md_buf_len);
    EVP_DigestFinal(&md_ctx, md_buf, &md_buf_len);

    token_len = akamaiBin2Hex(md_buf, 16, (unsigned char*)output);
    output[URL_TOKEN_LEN] = '\0';
    return 0;

}


/**
21.20.2.52 - - [01/Apr/2014 08:36:56] "POST /Apple/applefeed/dash-cal-b/Stream1_track_2-1396354390.mp4?akamai_token2=1396354469_6c0e676d309021d29237d6a789f62099:1234567890 HTTP/1.1" 200 -
*/



int main(void)
{
  std::string remote_path = "/Apple/applefeed/dash-cal-b/Stream1_track_2-1396354390.mp4";
  std::string m_akamaiToken = "akamai_token2";
  std::string m_akamaiSalt = "1234567890";

  time_t expires = 1396354469;
  int m_akamaiExpirationWindow = 300;

  time_t start_time = expires - m_akamaiExpirationWindow;
  char *extract = NULL;
  char *token = NULL;
  int err=0;

  if ((err = urlauth_gen_token(remote_path.c_str(), start_time, m_akamaiExpirationWindow, m_akamaiSalt.c_str(), extract, &expires, &token)) < 0)  {
    std::cout << "Failed to generated akamai token, err= " << err << std::endl;
    return -1;
  }
  else  {
    std::cout << "Generated akamai token = " << token << std::endl;
    return 0;
  }

}
