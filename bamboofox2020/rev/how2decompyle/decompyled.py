# uncompyle6 version 3.6.2
# Python bytecode 2.7 (62211)
# Decompiled from: Python 3.6.8 (default, Aug  7 2019, 17:28:10) 
# [GCC 4.8.5 20150623 (Red Hat 4.8.5-39)]
# Embedded file name: decompyle.py
# Compiled at: 2019-09-22 20:18:03
import string
restrictions = [
 'uudcjkllpuqngqwbujnbhobowpx_kdkp_',
 'f_negcqevyxmauuhthijbwhpjbvalnhnm',
 'dsafqqwxaqtstghrfbxzp_x_xo_kzqxck',
 'mdmqs_tfxbwisprcjutkrsogarmijtcls',
 'kvpsbdddqcyuzrgdomvnmlaymnlbegnur',
 'oykgmfa_cmroybxsgwktlzfitgagwxawu',
 'ewxbxogihhmknjcpbymdxqljvsspnvzfv',
 'izjwevjzooutelioqrbggatwkqfcuzwin',
 'xtbifb_vzsilvyjmyqsxdkrrqwyyiu_vb',
 'watartiplxa_ktzn_ouwzndcrfutffyzd',
 'rqzhdgfhdnbpmomakleqfpmxetpwpobgj',
 'qggdzxprwisr_vkkipgftuvhsizlc_pbz',
 'jerzhlnsegcaqzathfpuufwunakdtceqw',
 'lbvlyyrugffgrwo_v_zrqvqszchqrrljq',
 'aiwuuhzbszvfpidwwkl_wynlujbsbhfox',
 'vmhrizxtiegxdxsqcdoiyxkffloudwtxg',
 'tffjnabob_jbf_qiszdsemczghnjysmah',
 'zrqkppvynlkelnevngwlkhgaputhoagtt',
 'nl_oojyafwoqccbedijmigpedkdzglq_f',
 'cksy_skctjlyxktuzchvstunyvcvabomc',
 'ppcxleeguvhvhengmvac_bykhzqohjuei',
 '_clmaicjrrzhwd_fescyaejtbyefxyihy',
 'hhopvwsmjtpjiffzatyhjrev_dwnsidyo',
 'sjevtrmkkk_zjalxrxfovjsbcxjx_pskp',
 'gnynwuuqypddbsylparpcczqimimqmvdl',
 'bxitcmhnmanwuhvjxnqeoiimlegrmkjra']
capital = [
 0, 4, 9, 19, 23, 26]
flag = raw_input('Please tell me something : ').lower()
flag = flag.lower()
if len(flag) != len(restrictions[0]):
    print 'No......You are wrong orzzzzz'
    exit(0)
for f in range(len(flag)):
    for r in restrictions:
        if flag[f] not in string.lowercase + '_' or flag[f] == r[f]:
            print 'No......You are wrong orzzzzzzzzzzzz'
            exit(0)

cap_flag = ''
for f in range(len(flag)):
    if f in capital:
        cap_flag += flag[f].upper()
    else:
        cap_flag += flag[f]

print 'Yeah, you got it !\nBambooFox{' + cap_flag + '}\n'
# okay decompiling decompyle.pyc
