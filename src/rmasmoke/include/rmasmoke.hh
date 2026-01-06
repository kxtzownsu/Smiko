#ifndef __RMASMOKE_H
#define __RMASMOKE_H

struct tpm_result {
	__be16 tag;
	__be32 length;
	__be32 ordinal;
	__be16 subcmd;
	union {
		struct {
			__be32 digest;
			__be32 address;
			char data[0];
		} upgrade;
		struct {
			char data[0];
		} command;
	};
} __attribute__((packed));

// File-related functions.
int ReadFileToString(std::string path, std::string *data);
void *read_file(const char *path, size_t *size_ptr);

// Function for sending constructed TPM packets to the TPM.
uint32_t SendTPMCommand(tpm_result *buf, size_t len, tpm_result **respBuf,
                        size_t *respLen, int tpmfd);

/* TPM ownership related functions. */
TSS2L_SYS_AUTH_COMMAND generate_auth_token(void);
TSS2L_SYS_AUTH_COMMAND start_auth_session(void);
int tpm_set_passwords(std::string owner_password,
                      std::string endorsement_password,
					  std::string lockout_password);
std::string generate_rand(int size);
int tpm_take_ownership(void);

/* Memory control-related functions for the Cr50. */
uint32_t index_offset(uint32_t index);

void execute_shellcode(struct rmasmoke_regs_single *initial_regs, 
                       const void *data, size_t size);

void *cr50_memset(uint32_t addr, uint32_t val, 
                  uint32_t len);

void *cr50_memcpy(uint32_t dest_addr, uint32_t src_addr, 
                  uint32_t len);

// Function show the help menu.
void show_info(int esc);
#endif /* __RMASMOKE_H */