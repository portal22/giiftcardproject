/*
 * Gift Card Reading Application
 * Original Author: Shoddycorp's Cut-Rate Contracting
 * Comments added by: Justin Cappos (JAC) and Brendan Dolan-Gavitt (BDG)
 * Maintainer:
 * Date: 8 July 2020
 */


#include "giftcard.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

// .,~==== interpreter for THX-1138 assembly ====~,.
//
// This is an emulated version of a microcontroller with
// 16 registers, one flag (the zero flag), and display
// functionality. Programs can operate on the message
// buffer and use opcode 0x07 to update the display, so
// that animated greetings can be created.
void animate(char *msg, unsigned char *program) {
    unsigned char regs[16];
    char *mptr = msg;
    unsigned char *pc = program;
    int zf = 0;
    while (pc < program + 256) {
        unsigned char op, arg1, arg2;
        op = *pc;
        arg1 = *(pc + 1);
        arg2 = *(pc + 2);
        switch (op) {
            case 0x00:
                break;
            case 0x01:
                regs[arg1] = *mptr;
                break;
            case 0x02:
                *mptr = regs[arg1];
                break;
            case 0x03:
                mptr += (char)arg1;
                if (mptr < msg || mptr >= msg + 32) {
                    fprintf(stderr, "Error: Message pointer out of bounds\n");
                    return;
                }
                break;
            case 0x04:
                regs[arg2] = arg1;
                break;
            case 0x05:
                regs[arg1] ^= regs[arg2];
                zf = !regs[arg1];
                break;
            case 0x06:
                regs[arg1] += regs[arg2];
                zf = !regs[arg1];
                break;
            case 0x07:
                puts(msg);
                break;
            case 0x08:
                goto done;
            case 0x09:
                pc += (char)arg1;
                break;
            case 0x10:
                if (zf) pc += (char)arg1;
                break;
            default:
                fprintf(stderr, "Error: Unknown opcode %x\n", op);
                return;
        }
        pc += 3;
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
        usleep(5000);
#endif
    }
done:
    return;
}

int get_gift_card_value(struct this_gift_card *thisone) {
	struct gift_card_data *gcd_ptr;
	struct gift_card_record_data *gcrd_ptr;
	struct gift_card_amount_change *gcac_ptr;
	int ret_count = 0;

	gcd_ptr = thisone->gift_card_data;
	for(int i=0;i<gcd_ptr->number_of_gift_card_records; i++) {
  		gcrd_ptr = (struct gift_card_record_data *) gcd_ptr->gift_card_record_data[i];
		if (gcrd_ptr->type_of_record == 1) {
			gcac_ptr = gcrd_ptr->actual_record;
			ret_count += gcac_ptr->amount_added;
		}
	}
	return ret_count;
}

void print_gift_card_info(struct this_gift_card *thisone) {
	struct gift_card_data *gcd_ptr;
	struct gift_card_record_data *gcrd_ptr;
	struct gift_card_amount_change *gcac_ptr;
    struct gift_card_program *gcp_ptr;

	gcd_ptr = thisone->gift_card_data;
	printf("   Merchant ID: %32.32s\n",gcd_ptr->merchant_id);
	printf("   Customer ID: %32.32s\n",gcd_ptr->customer_id);
	printf("   Num records: %d\n",gcd_ptr->number_of_gift_card_records);
	for(int i=0;i<gcd_ptr->number_of_gift_card_records; i++) {
  		gcrd_ptr = (struct gift_card_record_data *) gcd_ptr->gift_card_record_data[i];
		if (gcrd_ptr->type_of_record == 1) {
			printf("      record_type: amount_change\n");
			gcac_ptr = gcrd_ptr->actual_record;
			printf("      amount_added: %d\n",gcac_ptr->amount_added);
			if (gcac_ptr->amount_added>0) {
				printf("      signature: %32.32s\n",gcac_ptr->actual_signature);
			}
		}
		else if (gcrd_ptr->type_of_record == 2) {
			printf("      record_type: message\n");
			printf("      message: %s\n",(char *)gcrd_ptr->actual_record);
		}
		else if (gcrd_ptr->type_of_record == 3) {
            gcp_ptr = gcrd_ptr->actual_record;
			printf("      record_type: animated message\n");
            // BDG: Hmm... is message guaranteed to be null-terminated?
            printf("      message: %s\n", gcp_ptr->message);
            printf("  [running embedded program]  \n");
            animate(gcp_ptr->message, gcp_ptr->program);
		}
	}
	printf("  Total value: %d\n\n",get_gift_card_value(thisone));
}

// Added to support web functionalities
void gift_card_json(struct this_gift_card *thisone) {
    struct gift_card_data *gcd_ptr;
    struct gift_card_record_data *gcrd_ptr;
    struct gift_card_amount_change *gcac_ptr;
    gcd_ptr = thisone->gift_card_data;
    printf("{\n");
    printf("  \"merchant_id\": \"%32.32s\",\n", gcd_ptr->merchant_id);
    printf("  \"customer_id\": \"%32.32s\",\n", gcd_ptr->customer_id);
    printf("  \"total_value\": %d,\n", get_gift_card_value(thisone));
    printf("  \"records\": [\n");
	for(int i=0;i<gcd_ptr->number_of_gift_card_records; i++) {
        gcrd_ptr = (struct gift_card_record_data *) gcd_ptr->gift_card_record_data[i];
        printf("    {\n");
        if (gcrd_ptr->type_of_record == 1) {
            printf("      \"record_type\": \"amount_change\",\n");
            gcac_ptr = gcrd_ptr->actual_record;
            printf("      \"amount_added\": %d,\n",gcac_ptr->amount_added);
            if (gcac_ptr->amount_added>0) {
                printf("      \"signature\": \"%32.32s\"\n",gcac_ptr->actual_signature);
            }
        }
        else if (gcrd_ptr->type_of_record == 2) {
			printf("      \"record_type\": \"message\",\n");
			printf("      \"message\": \"%s\"\n",(char *)gcrd_ptr->actual_record);
        }
        else if (gcrd_ptr->type_of_record == 3) {
            struct gift_card_program *gcp = gcrd_ptr->actual_record;
			printf("      \"record_type\": \"animated message\",\n");
			printf("      \"message\": \"%s\",\n",gcp->message);
            // programs are binary so we will hex for the json
            char *hexchars = "01234567890abcdef";
            char program_hex[512+1];
            program_hex[512] = '\0';
            int i;
            for(i = 0; i < 256; i++) {
                program_hex[i*2] = hexchars[((gcp->program[i] & 0xf0) >> 4)];
                program_hex[i*2+1] = hexchars[(gcp->program[i] & 0x0f)];
            }
			printf("      \"program\": \"%s\"\n",program_hex);
        }
        if (i < gcd_ptr->number_of_gift_card_records-1)
            printf("    },\n");
        else
            printf("    }\n");
    }
    printf("  ]\n");
    printf("}\n");
}


struct this_gift_card *gift_card_reader(FILE *input_fd) {
    struct this_gift_card *ret_val = malloc(sizeof(struct this_gift_card));
    if (!ret_val) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(1);
    }
    void *ptr;

    while (!feof(input_fd)) {
        struct gift_card_data *gcd_ptr;
        fread(&ret_val->num_bytes, 4, 1, input_fd);

        // Check for invalid num_bytes value
        if (ret_val->num_bytes <= 0 || ret_val->num_bytes > 1024 * 1024) {
            fprintf(stderr, "Error: Invalid number of bytes\n");
            exit(1);
        }

        ptr = malloc(ret_val->num_bytes);
        if (!ptr) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            exit(1);
        }
        fread(ptr, ret_val->num_bytes, 1, input_fd);

        gcd_ptr = ret_val->gift_card_data = malloc(sizeof(struct gift_card_data));
        if (!gcd_ptr) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            exit(1);
        }
        gcd_ptr->merchant_id = ptr;
        ptr += 32;
        gcd_ptr->customer_id = ptr;
        ptr += 32;
        gcd_ptr->number_of_gift_card_records = *((int *)ptr);
        ptr += 4;

        // Check for invalid number_of_gift_card_records value
        if (gcd_ptr->number_of_gift_card_records < 0 || gcd_ptr->number_of_gift_card_records > 100) {
            fprintf(stderr, "Error: Invalid number of gift card records\n");
            exit(1);
        }

        gcd_ptr->gift_card_record_data = malloc(gcd_ptr->number_of_gift_card_records * sizeof(void *));
        if (!gcd_ptr->gift_card_record_data) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            exit(1);
        }

        for (int i = 0; i < gcd_ptr->number_of_gift_card_records; i++) {
            struct gift_card_record_data *gcrd_ptr;
            gcrd_ptr = gcd_ptr->gift_card_record_data[i] = malloc(sizeof(struct gift_card_record_data));
            if (!gcrd_ptr) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                exit(1);
            }
            struct gift_card_amount_change *gcac_ptr;
            gcac_ptr = gcrd_ptr->actual_record = malloc(sizeof(struct gift_card_amount_change));
            if (!gcac_ptr) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                exit(1);
            }
            struct gift_card_program *gcp_ptr;
            gcp_ptr = malloc(sizeof(struct gift_card_program));
            if (!gcp_ptr) {
                fprintf(stderr, "Error: Memory allocation failed\n");
                exit(1);
            }

            gcrd_ptr->record_size_in_bytes = *((int *)ptr);
            ptr += 4;
            gcrd_ptr->type_of_record = *((int *)ptr);
            ptr += 4;

            if (gcrd_ptr->type_of_record == 1) {
                gcac_ptr->amount_added = *((int *)ptr);
                ptr += 4;

                if (gcac_ptr->amount_added < 0) break;

                gcac_ptr->actual_signature = ptr;
                ptr += 32;
            } else if (gcrd_ptr->type_of_record == 2) {
                gcrd_ptr->actual_record = ptr;
                ptr += strlen((char *)gcrd_ptr->actual_record) + 1;
            } else if (gcrd_ptr->type_of_record == 3) {
                gcp_ptr->message = malloc(32);
                gcp_ptr->program = malloc(256);
                if (!gcp_ptr->message || !gcp_ptr->program) {
                    fprintf(stderr, "Error: Memory allocation failed\n");
                    exit(1);
                }
                memcpy(gcp_ptr->message, ptr, 32);
                ptr += 32;
                memcpy(gcp_ptr->program, ptr, 256);
                ptr += 256;
                gcrd_ptr->actual_record = gcp_ptr;
            } else {
                fprintf(stderr, "Unknown record type: %d\n", gcrd_ptr->type_of_record);
                exit(1);
            }
        }
    }
    return ret_val;
}

struct this_gift_card *thisone;

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <1|2> file.gft\n", argv[0]);
        fprintf(stderr, "  - Use 1 for text output, 2 for JSON output\n");
        return 1;
    }
	FILE *input_fd = fopen(argv[2],"r");
    if (!input_fd) {
        fprintf(stderr, "error opening file\n");
        return 1;
    }
	thisone = gift_card_reader(input_fd);
	if (argv[1][0] == '1') print_gift_card_info(thisone);
    else if (argv[1][0] == '2') gift_card_json(thisone);

	return 0;
}