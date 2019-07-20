from hashlib import md5,sha256
# from secret import FLAG
FLAG=''
cand = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPWRSTUVWXYZ1234567890@,- _{}'

def gen():
    md5s = []
    sha256s = []
    for f in FLAG :
        assert f in cand
        md5s.append( int(md5(f.encode()).hexdigest(),16)%64 )
        sha256s.append( int(sha256(f.encode()).hexdigest(),16)%64 )

def solve():
    md5s = [41, 63, 46, 51, 6, 26, 42, 50, 44, 33, 29, 50, 27, 28, 30, 17, 31, 19, 46, 50, 33, 45, 26, 26, 29, 31, 52, 33, 1, 45, 31, 22, 50, 50, 50, 50, 50, 31, 22, 50, 44, 26, 44, 49, 50, 49, 26, 45, 31, 30, 22, 44, 30, 31, 17, 50, 50, 50, 31, 43, 52, 50, 53, 31, 30, 17, 26, 31, 46, 41, 44, 26, 31, 52, 50, 30, 31, 26, 39, 31, 46, 33, 27, 1, 42, 50, 31, 30, 12, 26, 27, 52, 31, 30, 12, 31, 46, 26, 27, 14, 50, 31, 22, 52, 33, 31, 41, 50, 46, 31, 22, 23, 41, 31, 53, 26, 21, 31, 33, 30, 31, 19, 39, 51, 33, 30, 39, 51, 12, 58, 60, 31, 41, 33, 53, 31, 3, 17, 50, 31, 51, 26, 29, 52, 31, 33, 22, 26, 31, 41, 51, 54, 41, 29, 52, 31, 19, 23, 33, 30, 44, 26, 27, 38, 8, 50, 29, 15]
    sha256s = [61, 44, 3, 14, 22, 41, 43, 30, 49, 59, 58, 30, 11, 3, 24, 35, 40, 46, 3, 42, 59, 36, 41, 41, 41, 40, 9, 59, 23, 36, 40, 33, 42, 42, 42, 42, 42, 40, 44, 42, 49, 24, 49, 28, 42, 33, 24, 36, 40, 24, 33, 10, 24, 40, 35, 42, 42, 42, 40, 39, 9, 42, 3, 40, 24, 35, 24, 40, 3, 61, 49, 24, 40, 9, 42, 24, 40, 41, 17, 40, 12, 57, 11, 23, 43, 42, 40, 24, 18, 41, 11, 9, 40, 24, 18, 40, 3, 41, 11, 12, 42, 40, 44, 9, 59, 40, 61, 42, 3, 40, 44, 13, 61, 40, 3, 24, 29, 40, 59, 24, 40, 19, 18, 6, 59, 24, 18, 6, 22, 0, 39, 40, 61, 57, 3, 40, 17, 35, 42, 40, 58, 24, 58, 9, 40, 59, 44, 24, 40, 61, 48, 52, 61, 58, 9, 40, 19, 13, 59, 24, 53, 41, 11, 55, 55, 42, 58, 18]


    ans = []
    for i in range(len(md5s)):
        for f in cand:
            if (int(md5(f.encode()).hexdigest(),16)%64 == md5s[i]
                and int(sha256(f.encode()).hexdigest(),16)%64 == sha256s[i]):
                ans.append(f)
                break # IMPORTANT
    print(''.join(ans))
solve()

# without break:
# AIS3{0N_May_16th @Sead00g said Heeeee ReMEMBerEd tH4t heee UseD thE SAME set 0f On1iNe to01s to S01Ve Rsa AeS RCA DE5 at T-cat-cup, AnD 7he kEys aRE AlWWAys TCat2019Key}
# correct:
# AIS3{0N_May_16th @Sead00g said Heeeee ReMEMBerEd tH4t heee UseD thE SAME set 0f On1iNe to01s to S01Ve Rsa AeS RCA DE5 at T-cat-cup, AnD 7he kEys aRE AlWAys TCat2019Key}
