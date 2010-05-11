/* Name: pages.h
 * Project: uNikeE - Software Ethernet MAC and upper layers stack
 * Author: Dmitry Oparin aka Rst7/CBSIE
 * Creation Date: 25-Jan-2009
 * Copyright: (C)2008,2009 by Rst7/CBSIE
 * License: GNU GPL v3 (see http://www.gnu.org/licenses/gpl-3.0.txt)
 */


#define __compressed_data_attr __flash

extern const char __compressed_data_attr * __compressed_data_attr CHUNKS[];
extern const char __compressed_data_attr http_404[];
extern const char __compressed_data_attr http_302[];
extern const char __compressed_data_attr http_401[];
extern const char __compressed_data_attr http_401stale[];
extern const char __compressed_data_attr http_root_level1[];
extern const char __compressed_data_attr http_root_level2[];
extern const char __compressed_data_attr http_root_level3[];
extern const char __compressed_data_attr http_root[];
extern const char __compressed_data_attr http_302z[];
extern const char __compressed_data_attr http_302s[];
extern const char __compressed_data_attr http_z[];
extern const char __compressed_data_attr http_s[];
extern const char __compressed_data_attr http_eeprom_refr_button[];

enum PAGES_VARS
{
_radid0_= 212, 
_radid1_,
_radid2_,
_radid3_,
_radid4_,
_radid5_,
_radid6_,
_radid7_,
_radid8_,
 _sel0_,
 _sel1_,
 _sel2_,
 _sel3_,
 _sel4_,
 _sel5_,
 _sel6_,
 _sel7_,
 _sel8_,
 _sel9_,
 _vlanid0_,
 _vlanid1_,
 _vlanid2_,
 _vlanid3_,
 _vlanid4_,
 _vlanid5_,
 _vlanid6_,
 _vlanid7_,
 _vlanid8_,
 _vlanid9_,
 _binary_out_,
 _select_name_,
 _dec_select_name_,
 _root_level1_,
 _root_level2_,
 _root_level3_,
 _check_eeprom_restored_,
 _logval_,
 _voltage_,
 _on_off_,
 _nonce_,
 _svn_revision_,
 _is_checked_
};

