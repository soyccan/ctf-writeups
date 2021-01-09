Name: IEAIE
Value: 0
Description: We implemented the image encryption algorithm described in the paper
G.Ye,C.Pan,X.Huang,Z.Zhao,and J.He,“A chaotic image encryption algorithm based on information entropy,” International Journal of Bifurca- tion and Chaos, vol. 28, no. 1, p. art. no. 1850010, 2018.

There is also a paper indicating the flaws of the algorithm
Cryptanalysis of a Chaotic Image Encryption Algorithm Based on Information Entropy
The paper stated that many images on the internet have the same entropy. So, we give you image.bmp which has the same entropy as flag.bmp

Given these, it should be easy for you to decrypt the encrypted_flag.bmp, right?

[Download Link](https://balsnctf-challenges-2020.s3.amazonaws.com/IEAIE/5c238c7aad6e2bc8ea46ed70ab2a68350b227320f33b6a73161225550bfd9c54.zip)

The encryption services are running at:

`ieaie-us.balsnctf.com:7122`

`ieaie-ap.balsnctf.com:7122`

`ieaie-eu.balsnctf.com:7122`

All services are identical except the server locations (US East, Asia, Europe). Choose the one near to your geolocation to reduce the network latency.

Author: kevin47
