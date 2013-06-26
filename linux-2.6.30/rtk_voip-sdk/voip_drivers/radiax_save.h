#ifndef __RADIAX_SAVE_H__
#define __RADIAX_SAVE_H__

#include "rtk_voip.h"

#if !defined (AUDIOCODES_VOIP)

extern int save_radiax_reg(void);
extern int load_radiax_reg(void);

#else //AUDIOCODES_VOIP

struct pt_acc {
	unsigned long mdl_l;
	unsigned long mdl_h;
	unsigned long mdh_l;
	unsigned long mdh_h;
};

struct pt_radiax {
	/* Saved accumolators. */
	struct pt_acc acc[4];

	/* Other saved registers. */
	unsigned long mmd;
	unsigned long lps0;
	unsigned long lpe0;
	unsigned long lpc0;
};

extern Word32 save_radiax_s(struct pt_radiax *reg);
extern Word32 restore_radiax_s(struct pt_radiax *reg);

static inline void save_radiax_reg( struct pt_radiax *radiax_regs) {save_radiax_s(radiax_regs);};
static inline void restore_radiax_reg( struct pt_radiax *radiax_regs) {restore_radiax_s(radiax_regs);};

#endif //AUDIOCODES_VOIP

#endif /* __RADIAX_SAVE_H__ */

