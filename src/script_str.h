/** \file script_cmd.h
 * \brief <введите краткую информацию об этом файле здесь>
 * \par
 * <введите более подробную информацию об этом файле здесь>
 * \par \author ARV \par
 * \note <введите примечани€ здесь>
 * \n —хема:
 * \n \date	14 ма€ 2018 г.
 * \par
 * \version <верси€>.	\par
 * Copyright 2015 © ARV. All rights reserved.
 * \par
 * ƒл€ компил€ции требуетс€:\n
 * 	-# Atmel Toolchain 3.4.5 или более нова€ верси€
 *
 */

#ifndef SCRIPT_STR_H_
#define SCRIPT_STR_H_

#if !defined(_DEBUG_)
#define INCLUDE_SHIFT_CMD
#else
#warning DEBUG MODE: SHIFT COMMANDS DISABLED!
#endif

#define	SCR_FILE_EXT	".sc"
#define SCR_DIR			""
#define SCR_CFG_FILE	"play.cfg"
#define SCR_CFG_TF		"TF"
#define SCR_CFG_RP		"RP"
#define SCR_CFG_IB		"IB"
#define SCR_CFG_PS		"PS"
#define SCR_CFG_IG		"IG"
///
#define SCR_CFG_LR		"LR"


#define CMD_PB		"PB"
#define CMD_GB		"GB"
#define CMD_PF		"PF"
#define CMD_GF		"GF"
#define CMD_PC		"PC"
#define CMD_GC		"GC"
#define CMD_PM		"PM"
#define CMD_WT		"WT"
#define CMD_LV		"LV"
#define CMD_END		"END"
#define CMD_CLR		"CLR"
#define CMD_INF		"INF"
#define CMD_RST		"RST"
#define CMD_RPT		"RPT"
#define CMD_PNT		"PNT"
#define CMD_REV		"REV"
#define CMD_NEG		"NEG"
// новые команды!
#define CMD_RGC		"RGC"	/* random global color */
#define CMD_RPC		"RPC"	/* random paint color */
#define CMD_GI		"GI"	/* global index [color] */
#define CMD_PI		"PI"	/* paint index [color] */
#define CMD_FM		"FM"	/* fade map */
#define CMD_BM		"BM"	/* bright map */
#if defined(INCLUDE_SHIFT_CMD)
#define CMD_RLC		"RLC"	/* rotate left color */
#define CMD_RRC		"RRC"	/* rotate right color */
#define CMD_RLB		"RLB"	/* rotate left bright */
#define CMD_RRB		"RRB"	/* rotate right bright */
#define CMD_SLC		"SLC"	/* shift left color */
#define CMD_SRC		"SRC"	/* shift right color */
#define CMD_SLB		"SLB"	/* shift left bright */
#define CMD_SRB		"SRB"	/* shift right bright */
#endif


#define VAR_CHAR	'V'
#define LAST_VAR_CHAR	'F'

#define FUNC_TP		"TP"
#define FUNC_RP		"RP"
#define FUNC_RD		"RD"
// new func
#define FUNC_DP		"DP"	/* dark pixel (random) */

#endif /* SCRIPT_STR_H_ */
