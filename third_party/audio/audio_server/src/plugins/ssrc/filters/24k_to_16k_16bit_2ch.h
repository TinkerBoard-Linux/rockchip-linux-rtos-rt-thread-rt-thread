static const struct
{
    short sNumPolyPhases;
    short sSampleIncrement;
    short sNumTaps;
    short sCoefs[2][13];
} SRCFilter_0_6666_16bit_2ch =
{
    2,
    3,
    13,
    {
        { -147, -1441, -600, 2881, -2632, 1705, 21117, 12387, -3789, 1112, 1633, -1726, -659 },
        { -659, -1726, 1633, 1112, -3789, 12387, 21117, 1705, -2632, 2881, -600, -1441, -147 },
    }
};
