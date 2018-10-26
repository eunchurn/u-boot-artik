#include <common.h>

#define ISRAM_BASE		(0x02020000 + 0x100)
#define EXTERNAL_FUNC_ADDRESS	(ISRAM_BASE + 0x00A0)
#define EXT_HASH_ADDRESS	(ISRAM_BASE + 0x00E4)
#define EXT_GET_BOOT_TYPE	(EXTERNAL_FUNC_ADDRESS + 0x08)
#define EXT_GET_DOWNLOADINGSIZE_ADDRESS (EXTERNAL_FUNC_ADDRESS + 0x38)

#define BL1_ADDRESS		(ISRAM_BASE - 0x100 + 0x1800)
#define BL2_ADDRESS		(ISRAM_BASE + 0x0120)
#define BL2_LOADING_BYTES	(ISRAM_BASE + 0x0118)
#define IOS_ADDRESS		(ISRAM_BASE + 0x012C)
#define IOS_LOADING_BYTES	(ISRAM_BASE + 0x0124)
#define IOS_CHECKSUM_REF	(ISRAM_BASE + 0x0128)

#define MB_REG_BASE		(0x800E0000)
#define MB_STATUS		(*(volatile u32 *)(MB_REG_BASE + 0x0000))

#define CTRL_FIELD_BASE		(MB_REG_BASE + 0x0100)

#define CTRL_FIELD_ADDR(val)	(CTRL_FIELD_BASE+(val<<2))
#define CTRL_FIELD(val)		(*(volatile u32 *)(CTRL_FIELD_ADDR(val)))

#define ISP_CTRL_FIELD_SET(index, value) 	CTRL_FIELD(index) = value
#define ISP_CTRL_FIELD_GET(index, value) 	value = CTRL_FIELD(index)

#define FUNC_SYSTEM_GET_INFO	0x0102
#define FUNC_SYSTEM_LOCK		0x0502

#define MB_SUCCESS		0
#define MB_FAIL			1

#define verify_pss_rsa_signature2(ptr, a, b, c, d, e, f, g, h)		\
	(((int(*)(int, int, unsigned char *, int, unsigned char *, int,	\
		unsigned char *, int))(((unsigned int *)(ptr))))	\
		(a, b, c, d, e, f, g, h))

#define ace_hash_sha_digest(out, buf, len, alg)				\
	(((int(*)(u8*, u8*, u32, s32))(*((u32 *)EXT_HASH_ADDRESS)))	\
		(out, buf, len, alg))

#define GetDownloadingSize()						\
	(((u32(*)(void))(*((u32 *)EXT_GET_DOWNLOADINGSIZE_ADDRESS)))())

#define is_sboot_enabled() (((u32(*)(void))(*((u32 *)EXT_GET_BOOT_TYPE)))())

/* # of sectors of OS image including header and signature */
#define MIN_IMAGE_SECTORS	3
#define MAX_IMAGE_SECTORS	16368

#define BL1_SIGNATURE_SIZE	1024
#define IMAGE_HEADER_SIZE	32
#define SIGNATURE_SIZE		272
#define SIGN_16BYTE_INFO	16
#define OS_VERIFY_KEY_SIZE	272

#define SB30_MAX_RSA_KEY	(2048 / 8)
#define SB30_MAX_SIGN_LEN	SB30_MAX_RSA_KEY
#define SB30_HMAC_SHA256_LEN	32

#define SB_OK			0x00000000
#define SB_OFF			0x80000000

struct rsa_public_key {
	int			rsa_n_Len;
	unsigned char		rsa_n[SB30_MAX_RSA_KEY];
	int			rsa_e_Len;
	unsigned char		rsa_e[4];
};

struct public_key_info {
	struct rsa_public_key	rsa_public_key;
	unsigned char		efuse[SB30_HMAC_SHA256_LEN];
};

struct sb30_info {
	unsigned int		codesignerversion;
	unsigned int		ap_info;
	unsigned long long	time;
	unsigned int		build_count;
	unsigned int		signing_type;
	unsigned char		description[32];
};

struct sb30_context {
	struct sb30_info	context_info;
	struct rsa_public_key	rsa_public_key;
	void			*func_vector[32];
	unsigned short		major_id;
	unsigned short		sub_id;
	unsigned char		reserved[8];
	struct public_key_info	public_key_info;
	int			code_signed_data_len;
	unsigned char		code_signed_data[SB30_MAX_SIGN_LEN];
};

enum {
	RSA,
	ECDSA256
};

enum {
	ALG_SHA1,
	ALG_SHA256,
};

static u32 get_image_base(void)
{
	return readl(IOS_ADDRESS);
}

static void set_image_base(u32 addr)
{
	writel(addr, IOS_ADDRESS);
}

static u32 get_bl2image_base(void)
{
	return readl(BL2_ADDRESS);
}

static u32 get_image_checksum(void)
{
	return readl(IOS_CHECKSUM_REF);
}

static void set_image_checksum(u32 checksum)
{
	writel(checksum, IOS_CHECKSUM_REF);
}

static u32 get_image_size(void)
{
	return readl(IOS_LOADING_BYTES);
}

static u32 get_bl2image_size(void)
{
	return readl(BL2_LOADING_BYTES);
}

static void set_image_size(u32 bytes)
{
	writel(bytes, IOS_LOADING_BYTES);
}

struct img_header {
	unsigned int	nsectors;
	unsigned int	checksum;
	unsigned int	magic1;
	unsigned int	reserved[5];
};

static int parse_img_header(void)
{
	struct img_header *h = (struct img_header *)get_image_base();

	if (h->nsectors > MAX_IMAGE_SECTORS || h->nsectors < MIN_IMAGE_SECTORS)
		return 0;

	set_image_size(h->nsectors << 9);
	set_image_checksum(h->checksum);

	return 1;
}

int direct_access_lock(void)
{
	int ret = 0;

	if (MB_STATUS) {
		return MB_FAIL;
	}

	ISP_CTRL_FIELD_SET(0, FUNC_SYSTEM_GET_INFO);

	while((MB_STATUS) & (0x01))
	ISP_CTRL_FIELD_GET(0, ret);
	if (ret != 0xA1) {
		printf("%s FUNC_SYSTEM_GET_INFO Fail:0x%x\n", __func__, ret);
		return MB_FAIL;
	}

	ISP_CTRL_FIELD_SET(0, FUNC_SYSTEM_LOCK);
	while((MB_STATUS) & (0x01))

	ISP_CTRL_FIELD_GET(0, ret);
	if (ret != 0xA1) {
		printf("%s FUNC_SYSTEM_LOCK Fail:0x%x\n", __func__, ret);
		return MB_FAIL;
	}

	return MB_SUCCESS;
}

/*
 * Verify os with rsa public key in bl2 context
 */
static int check_rsa_signature(struct sb30_context *context,
		unsigned char *start, int len,
		unsigned char *signature, int siglen)
{
	u32 rsa_pub;

	rsa_pub = get_bl2image_base() + get_bl2image_size()
			- SIGNATURE_SIZE - OS_VERIFY_KEY_SIZE;

	return verify_pss_rsa_signature2(
			context->func_vector[1], 1, 1,
			(unsigned char *)rsa_pub,
			sizeof(struct rsa_public_key), start, len,
			(unsigned char *)signature, siglen) == SB_OK;
}

static int verify_signature(void)
{
	u32 context;
	u32 start;
	u32 length;
	u32 signature;

	start = get_image_base();

	/*
	 * Get pure code length
	 * Read BL2 header to get BL2 size
	 *   1 sector  == 512 Bytes
	 *   272 Bytes == signature
	 */
	length = get_image_size() - SIGNATURE_SIZE + SIGN_16BYTE_INFO;

	/* Get sign code address */
	signature = start + length;

	/* 0x02023400 for 8KB bl1 */
	context = BL1_ADDRESS + GetDownloadingSize() - BL1_SIGNATURE_SIZE;

	return check_rsa_signature((struct sb30_context *)context,
				(unsigned char *)start,
				length,
				(unsigned char *)signature,
				SB30_MAX_SIGN_LEN);
}

static int verify_checksum(void)
{
	u32 start;
	u32 length;
	u32 checksum;
	u8 hash[32];

	start  = get_image_base() + IMAGE_HEADER_SIZE;
	length = get_image_size() - IMAGE_HEADER_SIZE - SIGNATURE_SIZE;

	ace_hash_sha_digest(hash, (u8 *)start, length, ALG_SHA256);

	checksum = hash[0] + (hash[1] << 8) + (hash[2] << 16) + (hash[3] << 24);

	/* we check only first 4 bytes of digest */
	return get_image_checksum() == checksum;
}

int authenticate_image(u32 baseaddr)
{
	set_image_base(baseaddr);

	if (!parse_img_header())
		return 0;

	if (verify_checksum() && (!is_sboot_enabled() || verify_signature()))
		return 1;

	return 0;
}

int is_sboot(void)
{
	return is_sboot_enabled();
}

static int do_sss_status(cmd_tbl_t *cmdtp, int flag, int argc,
		char *const argv[])
{
	if (argc != 1)
		return CMD_RET_USAGE;

	if (is_sboot_enabled()) {
		puts("SBOOT: Enabled\n");
		return CMD_RET_SUCCESS;
	}

	return CMD_RET_FAILURE;
}

static int do_authenticate_image(cmd_tbl_t *cmdtp, int flag, int argc,
		char *const argv[])
{
	u32 addr;

	if (argc != 2)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], NULL, 16);

	if (authenticate_image(addr))
		return CMD_RET_SUCCESS;

	return CMD_RET_FAILURE;
}

U_BOOT_CMD(
		sss_status, CONFIG_SYS_MAXARGS, 1, do_sss_status,
		"display SSS status and configuraions (if any)",
		""
	  );

U_BOOT_CMD(
		sss_auth_img, 2, 1, do_authenticate_image,
		"authenticate image via SSS",
		"addr\n"
		"addr - image hex address\n"
	  );
