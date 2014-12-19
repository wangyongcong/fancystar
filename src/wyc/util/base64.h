#ifndef __HADER_WYC_BASE64
#define __HADER_WYC_BASE64

namespace wyc
{

char *base64_encode(const char *data, size_t input_length, size_t *output_length);
char *base64_decode(const char *data, size_t input_length, size_t *output_length);
void base64_free(char *str);
void base64_cleanup();

}; // namespace wyc

#endif //__HADER_WYC_BASE64