#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(void)
{
	uint8_t name[4] = "CSCO";

    char str[] = "<inletf1><channel><output format=\"hds\"><output_id>0bf6f491-741c-4cd8-ad02-ce07817571a5</output_id><output_admin_state>Enabled</output_admin_state><output_stream><stream_id>a1e9a5ba-75cd-43e3-a005-c0f883861d19</stream_id><input_id type=\"video\">0c0376e8-3688-4016-88a4-5018aa3c5ddd</input_id><input_id type=\"audio\" label=\"English\">f67af76b-87bf-4ea0-b061-d4be5d0fcb58</input_id><input_id type=\"metadata\">a68d26cc-0551-457b-9332-e29cb3601aa6</input_id><hds_streamname>StreamA</hds_streamname></output_stream><output_stream><stream_id>8247e13d-7d56-42c6-b50f-ec6a24850d0f</stream_id><input_id type=\"video\">02a5967a-7ab4-4cee-9519-03cbda2f212e</input_id><input_id type=\"audio\" label=\"English\">79d6c3a8-7e0c-4585-82ff-4d49468b54e1</input_id><input_id type=\"metadata\">948be276-7b9a-48a7-a7a5-d45a3ebc09e1</input_id><hds_streamname>StreamB</hds_streamname></output_stream><output_stream><stream_id>141571f9-7a95-4040-8fbb-151f63bdd470</stream_id><input_id type=\"video\">eea37517-9d92-47b4-8712-32e99b7ac9a0</input_id><input_id type=\"audio\" label=\"English\">498c8f5e-353e-4b98-b2e3-dffc12c87b0a</input_id><input_id type=\"metadata\">788b18f4-3849-4f4b-a28d-670dca3d5cc8</input_id><hds_streamname>StreamC</hds_streamname></output_stream><output_stream><stream_id>40d97486-6bfa-4dee-804c-6f1688f1e38b</stream_id><input_id type=\"video\">50c18847-35f1-49fe-8822-8f7e07660f93</input_id><input_id type=\"audio\" label=\"English\">39b33910-f38f-4f2e-8bfb-caf3cf09efc3</input_id><input_id type=\"metadata\">29c27e42-a080-4e65-bc18-632757682d6d</input_id><hds_streamname>StreamD</hds_streamname></output_stream><output_stream><stream_id>74377b18-90f1-4768-957a-c2b366f9f326</stream_id><input_id type=\"video\">e5b8bd3b-f0b8-43fc-b450-2cb0af33a172</input_id><input_id type=\"audio\" label=\"English\">99d83324-98da-4efe-96ee-a96d05b6a5fd</input_id><input_id type=\"metadata\">6ece4947-7caf-42ac-83e1-1d97f929614a</input_id><hds_streamname>StreamE</hds_st";

	printf("name[]=%d, %d, %d, %d\n", name[0], name[1], name[2], name[3]); 	

    printf("str=%s, len=%d\n", str, strlen(str));
    return 0;
}
