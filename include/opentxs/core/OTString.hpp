/************************************************************
 *
 *  OTString.hpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#ifndef __OT_STRING_HPP__
#define __OT_STRING_HPP__

#include "OTCommon.hpp"

#ifdef _WIN32
#include "win32_utf8conv.hpp" // support for changing between std::string and std::wstring
#endif

#include <list>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

#include <cstdarg>
#include <cstring>

// All of the below PRI values are defined in inttypes.h
// Therefore if it's NOT defined, then we must probably be
// on Windows, since Windows doesn't have inttypes.h yet,
// only stdint.h
#if !defined(PRId8)
#define PRId8 "d"
#endif

#if !defined(PRId16)
#define PRId16 "d"
#endif

#if !defined(PRId32)
#if defined(WINAPI)
#define PRId32 "I32d"

#else
#define PRId32 "d"

#endif
#endif

#if !defined(PRId64)
#if defined(WINAPI)
#define PRId64 "I64d"

#elif __WORDSIZE == 64
#define PRId64 "ld"

#else
#define PRId64 "lld"

#endif
#endif

//#ifdef _WIN32
//#define MAX_STRING_LENGTH   631072
//#else
#define MAX_STRING_LENGTH 0x800000 // this is about 8 megs.
//#define MAX_STRING_LENGTH   1262144
//#endif // _WIN32
// TODO: consider how MAX_SIZE affects the above hard-coded number...

namespace opentxs
{

class OTASCIIArmor;
class OTContract;
class OTIdentifier;
class OTPseudonym;
class OTSignature;

typedef std::list<std::string> listOfStrings;
typedef std::map<std::string, std::string> mapOfStrings;

class OTString
{
public:
    EXPORT friend std::ostream& operator<<(std::ostream& os,
                                           const OTString& obj);

    EXPORT OTString();
    EXPORT OTString(const OTString& strValue);
    EXPORT OTString(const OTASCIIArmor& strValue);
    OTString(const OTSignature& strValue);
    EXPORT OTString(const OTContract& theValue);
    EXPORT OTString(const OTIdentifier& theValue);
    OTString(OTPseudonym& theValue);
    EXPORT OTString(const char* new_string);
    OTString(const char* new_string, size_t sizeLength);
    EXPORT OTString(const std::string& new_string);
    EXPORT virtual ~OTString();

    void Initialize();

    EXPORT OTString& operator=(OTString rhs);

    static bool vformat(const char* fmt, std::va_list* pvl,
                        std::string& str_output);

    void swap(OTString& rhs);
    bool operator>(const OTString& s2) const;
    bool operator<(const OTString& s2) const;
    bool operator<=(const OTString& s2) const;
    bool operator>=(const OTString& s2) const;
    EXPORT bool operator==(const OTString& s2) const;

    EXPORT static std::string& trim(std::string& str);
    EXPORT static const std::string replace_chars(const std::string& str,
                                                  const std::string& charsFrom,
                                                  const char& charTo);
#ifdef _WIN32
    EXPORT static std::wstring s2ws(const std::string& s);
    EXPORT static std::string ws2s(const std::wstring& s);
#endif

public:
    EXPORT static bool safe_strcpy(
        char* dest, const char* src,
        size_t dest_size, // max size of destination must be passed here.
        bool bZeroSource = false); // if true, sets the source buffer to zero
                                   // after copying is done.
    static size_t safe_strlen(const char* s, size_t max);

    EXPORT static int64_t StringToLong(const std::string& strNumber);

    EXPORT int64_t ToLong() const;

    EXPORT static uint64_t StringToUlong(const std::string& strNumber);

    EXPORT uint64_t ToUlong() const;

    EXPORT bool At(uint32_t lIndex, char& c) const;
    EXPORT bool Exists() const;
    EXPORT bool DecodeIfArmored(bool bEscapedIsAllowed = true);
    EXPORT uint32_t GetLength() const;
    EXPORT bool Compare(const char* strCompare) const;
    EXPORT bool Compare(const OTString& strCompare) const;

    EXPORT bool Contains(const char* strCompare) const;
    bool Contains(const OTString& strCompare) const;

    EXPORT const char* Get() const;
    // new_string MUST be at least nEnforcedMaxLength in size if
    // nEnforcedMaxLength is passed in at all.
    //
    // That's because this function forces the null terminator at
    // that length, minus 1. For example, if the max is set to 10, then
    // the valid range is 0..9. Therefore 9 (10 minus 1) is where the
    // NULL terminator goes.
    //
    EXPORT void Set(const char* new_string, uint32_t nEnforcedMaxLength = 0);
    EXPORT void Set(const OTString& strBuf);
    // For a straight-across, exact-size copy of bytes.
    // Source not expected to be null-terminated.
    EXPORT bool MemSet(const char* pMem, uint32_t theSize);
    EXPORT void Concatenate(const char* arg, ...);
    void Concatenate(const OTString& strBuf);
    void Truncate(uint32_t lAt);
    EXPORT void Format(const char* fmt, ...);
    void ConvertToLowerCase();
    void ConvertToUpperCase();
    EXPORT bool TokenizeIntoKeyValuePairs(mapOfStrings& mapOutput) const;
    EXPORT void OTfgets(std::istream& ofs);
    // true  == there are more lines to read.
    // false == this is the last line. Like EOF.
    bool sgets(char* szBuffer, uint32_t nBufSize);

    char sgetc();
    void sungetc();
    void reset();

    void WriteToFile(std::ostream& ofs) const;
    EXPORT virtual void Release();
    void Release_String();
    EXPORT void zeroMemory();

private:
    // You better have called Initialize() or Release() before you dare call
    // this.
    void LowLevelSetStr(const OTString& strBuf);

    // Only call this right after calling Initialize() or Release().
    // Also, this function ASSUMES the new_string pointer is good.
    void LowLevelSet(const char* new_string, uint32_t nEnforcedMaxLength);

protected:
    uint32_t m_lLength;
    uint32_t m_lPosition;
    char* m_strBuffer;
};

// bool operator >(const OTString& s1, const OTString& s2);
// bool operator <(const OTString& s1, const OTString& s2);
// bool operator >=(const OTString &s1, const OTString& s2);
// bool operator <=(const OTString &s1, const OTString& s2);

} // namespace opentxs

#endif // __OT_STRING_HPP__
