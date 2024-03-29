/*
 Digest checksum tools:
 md5sum    - Print or check MD5 checksums. 
 sha1sum   - Print or check SHA1 checksums.
 sha256sum - Print or check SHA256 checksums.
 sha384sum - Print or check SHA384 checksums.
 sha512sum - Print or check SHA512 checksums.

 Written in C++ for Windows platform. All above tools share the same source
 code and only compile once and modify the .exe files' name to md5sum.exe,
 sha1sum.exe, sha256sum.exe etc.
 https://github.com/fshb/digest-checksum-tools/
 Copyright (c) 2019 Sun Hongbo (Felix)

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this Software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#pragma once

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <wincrypt.h>
#include "tstring.h"
#include "opt.h"

msg_handler helpmsgs;
msg_handler outs;
msg_handler errs(stderr);

void USAGE(const TCHAR* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	helpmsgs().vformat(fmt, ap);
	va_end(ap);
}
void PrintUsage()
{
	helpmsgs.print();
}
enum AlgHash : ALG_ID
{
	MD5 = CALG_MD5,
	SHA1 = CALG_SHA1,
	SHA256 = CALG_SHA_256,
	SHA384 = CALG_SHA_384,
	SHA512 = CALG_SHA_512,

	UNKNOWN_ALG
};
void Usage(int status);
struct global_options_struct
{
private:
	int binary_flag;

public:
	bool _binary;
	bool _warn;
	bool _do_check;
	bool _bsd_tag;
	bool _quiet;
	bool _status_only;
	bool _ignore_missing;
	bool _strict;
	str _delim; //zero? delimiter
	AlgHash _digest_alg;
	str _program_name;
	str _alg_lecture_ref;
	str _digest_alg_name;
	global_options_struct() : _binary(true), _do_check(false), _warn(false),
		_bsd_tag(false), _quiet(false), _status_only(false),
		_ignore_missing(false), _strict(false), _delim(_T("\n")),
		binary_flag(0), _digest_alg(MD5), _program_name(_T("md5sum")),
		_alg_lecture_ref(_T("RFC 1321")), _digest_alg_name(_T("MD5")) {}

	void InitMain(int argc, const TCHAR* argv[])
	{
		_program_name = argv[0];
		strs list = _program_name.split(_T("\\")); //split path string by '\' delimiter
		_program_name = *list.rbegin();
		list.clear();
		if (!_program_name.is_null())
		{
			str::size_type pos = _program_name.length() - 1;
			while (pos != 0) //remove program extension, e.g. ".exe"
			{
				if (_program_name[pos] == '.')
				{
					_program_name = _program_name.substr(0, pos);
					break;
				}
				pos--;
			}
			_program_name.to_lower();
		}
		list.clear();

		//to lower case
		_program_name.to_lower();
		
		if (_program_name == _T("md5sum"))
		{
			_digest_alg = MD5;
			_digest_alg_name = _T("MD5");
			_alg_lecture_ref = _T("RFC 1321");
		}
		else if (_program_name == _T("sha1sum"))
		{
			_digest_alg = SHA1;
			_digest_alg_name = _T("SHA1");
			_alg_lecture_ref = _T("FIPS-180-1");
		}
		else if (_program_name == _T("sha256sum"))
		{
			_digest_alg = SHA256;
			_digest_alg_name = _T("SHA256");
			_alg_lecture_ref = _T("FIPS-180-2");
		}
		else if (_program_name == _T("sha384sum"))
		{
			_digest_alg = SHA384;
			_digest_alg_name = _T("SHA384");
			_alg_lecture_ref = _T("FIPS-180-2");
		}
		else if (_program_name == _T("sha512sum"))
		{
			_digest_alg = SHA512;
			_digest_alg_name = _T("SHA512");
			_alg_lecture_ref = _T("FIPS-180-2");
		}
		else
		{
			_digest_alg = MD5;
			_digest_alg_name = _T("MD5");
			_alg_lecture_ref = _T("RFC 1321");
			_program_name = _T("md5sum");
		}

	}
	void SetBinaryFlag()
	{
		binary_flag++;
	}
	void DisposeOptionConflict() const
	{
		if (_bsd_tag && !_binary)
		{
			errs() << _T("--tag does not support --text mode");
			errs.print();
			Usage(EXIT_FAILURE);
		}
		if (_delim != _T("\n") && _do_check)
		{
			errs() << _T("the --zero option is not supported when verifying checksums");
			errs.print();
			Usage(EXIT_FAILURE);
		}

		if (_bsd_tag && _do_check)
		{
			errs() << _T("the --tag option is meaningless when verifying checksums");
			errs.print();
			Usage(EXIT_FAILURE);
		}

		if (binary_flag && _do_check)
		{
			errs() << _T("the --binary and --text options are meaningless when verifying checksums");
			errs.print();
			Usage(EXIT_FAILURE);
		}

		if (_ignore_missing && !_do_check)
		{
			errs() << _T("the --ignore-missing option is meaningful only when verifying checksums");
			errs.print();
			Usage(EXIT_FAILURE);
		}

		if (_status_only && !_do_check)
		{
			errs() << _T("the --status option is meaningful only when verifying checksums");
			errs.print();
			Usage(EXIT_FAILURE);
		}

		if (_warn && !_do_check)
		{
			errs() << _T("the --warn option is meaningful only when verifying checksums");
			errs.print();
			Usage(EXIT_FAILURE);
		}

		if (_quiet && !_do_check)
		{
			errs() << _T("the --quiet option is meaningful only when verifying checksums");
			errs.print();
			Usage(EXIT_FAILURE);
		}

		if (_strict & !_do_check)
		{
			errs() << _T("the --strict option is meaningful only when verifying checksums");
			errs.print();
			Usage(EXIT_FAILURE);
		}
	}

	void DisposeInvalidOption(bool haserr = false) const
	{
		if (!haserr)
			return;

		errs().format(_T("Try '%s --help' for more information."), _program_name.c_str());
		errs.print();
		exit(EXIT_FAILURE);
	}
} g_option;

void Usage(int status)
{
	USAGE(_T("Usage: %s [OPTION]... [FILE]..."), g_option._program_name.c_str());
	USAGE(_T("Print or check %s checksums."), g_option._digest_alg_name.c_str());
	USAGE(_T(""));
	USAGE(_T("With no FILE, or when FILE is '-', read standard input. "));
	USAGE(_T(""));
	USAGE(_T("  -b, --binary          read in binary mode (default)"));
	USAGE(_T("  -c, --check           read %s sums from the FILEs and check them"), g_option._digest_alg_name.c_str());
	USAGE(_T("      --tag             create a BSD-style checksum"));
	USAGE(_T("  -t, --text            read in text mode"));
	USAGE(_T(""));
	USAGE(_T("The following five options are useful only when verifying checksums:"));
	USAGE(_T("      --ignore-missing  don't fail or report status for missing files"));
	USAGE(_T("      --quiet           don't print OK for each successfully verified file"));
	USAGE(_T("      --status          don't output anything, status code shows success"));
	USAGE(_T("      --strict          exit non-zero for improperly formatted checksum lines"));
	USAGE(_T("  -w, --warn            warn about improperly formatted checksum lines"));
	USAGE(_T(""));
	USAGE(_T("      --help            display this help and exit"));
	USAGE(_T("      --version         output version information and exit"));
	USAGE(_T(""));
	USAGE(_T("The sums are computed as described in %s. ")
		_T("When checking, the input should be a former output of this program. ")
		_T("The default mode is to print a line with checksum, a space, a character indicating input mode ")
		_T("('*' for binary, ' ' for text or where binary is insignificant), and name for each FILE."), 
		g_option._alg_lecture_ref.c_str());
	PrintUsage();
	exit(status);
}

void Version()
{
	USAGE(_T("%s: version 1.0\n"), g_option._program_name.c_str());
	USAGE(_T("Author: Sun Hongbo (Felix), @2019\n"));
	PrintUsage();
	exit(EXIT_SUCCESS);
}

bool VerifyFile(str& zIn_FileToVerify)
{
	if (zIn_FileToVerify == _T("-"))
		return true;

	HANDLE hFind;
	WIN32_FIND_DATA a;

	hFind = FindFirstFile(zIn_FileToVerify.c_str(), &a);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	if (!(a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		FindClose(hFind);
		return true;
	}

	while (FindNextFile(hFind, &a))
	{
		if (!(a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			FindClose(hFind);
			return true;
		}
	}
	FindClose(hFind);

	return false;
}

void SplitFileName(str& zIn_FileNameToSplit, str& zOut_SplitedFilePath, str& zOut_SplitedFileName)
{
	size_t len = zIn_FileNameToSplit.length();
	size_t i = len;
	while (i != 0)
	{
		if (zIn_FileNameToSplit[i] == '\\')
		{
			zOut_SplitedFilePath = zIn_FileNameToSplit.substr(i + 1);
			zOut_SplitedFileName = zIn_FileNameToSplit.substr(0, i - 1);
			break;
		}
		i--;
	}
}

bool ParseFileName(std::vector<str>& zOut_ParsedFiles, str& zIn_FileToParse)
{
	if (zIn_FileToParse == _T("-"))
	{
		zOut_ParsedFiles.push_back(zIn_FileToParse);
		return true;
	}

	HANDLE hFind;
	WIN32_FIND_DATA a;

	hFind = FindFirstFile(zIn_FileToParse.c_str(), &a);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		errs().format(_T("%s: %s: no such file or directory"),
			g_option._program_name.c_str(), zIn_FileToParse.c_str());
		return false;
	}

	str zPath, zName;
	if (!(a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		SplitFileName(zIn_FileToParse, zPath, zName);
		zOut_ParsedFiles.push_back(zPath + a.cFileName);
	}
	while (FindNextFile(hFind, &a))
	{
		if (!(a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			SplitFileName(zIn_FileToParse, zPath, zName);
			zOut_ParsedFiles.push_back(zPath + a.cFileName);
		}
	}
	FindClose(hFind);

	return true;
}

bool ComputeFileDigest(str& zIn_FileToCompute, str& zOut_Digest, AlgHash alg_id, bool is_binary_mode)
{
	FILE* f = NULL;
	if (zIn_FileToCompute == _T("-"))
		f = stdin;
	else
		_tfopen_s(&f, zIn_FileToCompute.c_str(), is_binary_mode ? _T("rb") : _T("r"));

	if (f == NULL)
		return false;

	struct
	{
		bool operator()(TCHAR c, BYTE* buf, size_t max_buf_size, DWORD* n_bytes_transferred)
		{
			if (max_buf_size < sizeof(TCHAR))
				return false;
			int n = sizeof(TCHAR);
			for (int i = 0; i < n; i++)
			{
				buf[i] = (c >> (n - i - 1)) & 0xFF;
			}
			return true;
		}
	} _tchar2byte;

	HCRYPTPROV hProv;
	HCRYPTHASH hHash;

	//at least 64 bytes since SHA512 has the longest output (512 bits == 64 bytes)
	const size_t max_hash_data_bytes = 64;
	BYTE *pbHash = new BYTE[max_hash_data_bytes];

	const size_t max_buffer_size = 1024;//1KB buffer
	BYTE *pbBuffer = new BYTE[max_buffer_size];

	DWORD nBytesRead;
	DWORD dwHashLen = 0;

	//create CSP
	CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES/*use PROV_RSA_AES instead of PROV_RSA_FULL to support SHA2 algorithms*/, CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET);
	CryptCreateHash(hProv, alg_id, 0, 0, &hHash);

	do {
		if (!is_binary_mode)
		{
			if (!_tchar2byte(_fgettc(f), pbBuffer, max_buffer_size, &nBytesRead))
				break;
		}
		else
			nBytesRead = (DWORD) fread(pbBuffer, sizeof(BYTE), max_buffer_size, f);

		CryptHashData(hHash, pbBuffer, nBytesRead, 0);
	} while (!feof(f) && !ferror(f));

	fclose(f);

	CryptGetHashParam(hHash, HP_HASHVAL, NULL, &dwHashLen, 0);  //get dwHashLen
	CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &dwHashLen, 0); //get bHash

	DWORD i, k;

	//at least 128 + 1('\0') chars because SHA512 algorithm will generate 128 chars
	const size_t max_hash_str_len = 129;
	TCHAR *cHashStr = new TCHAR[max_hash_str_len];
	memset(cHashStr, 0, max_hash_str_len);

	static const TCHAR *HexDigits = _T("0123456789abcdef");
	for (i = 0; i < dwHashLen; i++)
	{
		k = pbHash[i] & 0xF;
		cHashStr[2 * i] = HexDigits[k]; //lower nibble

		k = pbHash[i] >> 4 & 0xF;
		cHashStr[2 * i + 1] = HexDigits[k]; //upper nibble
	}
	zOut_Digest = cHashStr;

	delete[] pbBuffer;
	delete[] pbHash;
	delete[] cHashStr;

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);

	return true;
}
bool IsHexDigit(TCHAR c)
{
	static const TCHAR *s = _T("0123456789abcdefABCDEF");
	int i = 0;
	while (s[i] != '\0')
	{
		if (c == s[i])
			return true;
		i++;
	}
	return false;
}

bool ParseLine(TCHAR* cLine, str& zOut_DigestInLine, str& zOut_FileNameInLine, bool& is_binary, AlgHash& alg_id)
{
	str zLine = cLine;
	size_t len = zLine.length();
	if (len < 32)
		return false;

	const size_t max_filename_len = 512;
	TCHAR* cFileName = new TCHAR[max_filename_len];
	memset(cFileName, '\0', max_filename_len * sizeof(TCHAR));

	//at least 128 + 1('\0') chars because SHA512 algorithm will generate 128 chars
	const size_t max_digest_chars = 129;
	TCHAR* cDigest = new TCHAR[max_digest_chars];
	memset(cDigest, '\0', max_digest_chars * sizeof(TCHAR));

	str::size_type i = 0;
	if (zLine.find(_T("MD5")) == 0 || zLine.find(_T("SHA")) == 0)// BSD style: '--tag'
	{
		//BSD style (doesn't support '--text' mode):
		//MD5 (file) = 05b04f4921652d0bc7dbf0835ba89fe1
		if (zLine.find(_T("MD5")) == 0)
		{
			alg_id = MD5; //32 hex digits
			i = 3;
		}
		else if (zLine.find(_T("SHA1")) == 0)
		{
			alg_id = SHA1; //40 hex digits
			i = 5;
		}
		else if (zLine.find(_T("SHA256")) == 0)
		{
			alg_id = SHA256; //64 hex digits
			i = 6;
		}
		else if (zLine.find(_T("SHA384")) == 0)
		{
			alg_id = SHA384; //96 hex digits
			i = 6;
		}
		else if (zLine.find(_T("SHA512")) == 0)
		{
			alg_id = SHA512; //128 hex digits
			i = 6;
		}
		else
		{
			alg_id = UNKNOWN_ALG;
		}

		while (i < len)
		{
			if (zLine[i] == '(')
			{
				i++;
				size_t k = 0;
				while (zLine[i] != ')' && zLine[i] != '\0') //read file name
				{
					cFileName[k] = zLine[i];
					i++;
					k++;
				}
				k = 0;
				while (zLine[i] != '\0')//read digest data
				{
					if (zLine[i] == '=' || zLine[i] == ' ')
						i++;
					else
					{
						if (zLine[i] != '\n' && IsHexDigit(zLine[i]))
						{
							cDigest[k] = zLine[i];
							k++;
						}
					}
				}
			}
			i++;
		}
		is_binary = false;//--tag does not support --text mode
	}
	else // GNU style
	{
		//GNU style:
		//05b04f4921652d0bc7dbf0835ba89fe1 *file
		//05b04f4921652d0bc7dbf0835ba89fe1  file
		i = 0;
		if (!IsHexDigit(zLine[0]))
		{
			delete[] cDigest;
			delete[] cFileName;
			return false;
		}
		size_t k = 0, j = 0;
		bool bEndReadDigest = false;
		bool bStartReadFilename = false;
		is_binary = false;
		while (i < len)
		{
			if (!bEndReadDigest && IsHexDigit(zLine[i]))//read digest data
			{
				cDigest[k] = zLine[i];
				k++;
			}
			else// read file name
			{
				bEndReadDigest = true;
				if (!bStartReadFilename)
				{
					if (zLine[i] != ' ')
						bStartReadFilename = true;
				}
				else if (zLine[i] != '\n')
				{
					cFileName[j] = zLine[i];
					j++;
					if (j == 0 && cFileName[j] == '*')
						is_binary = true;
				}
			}
			i++;
		}
		i = _tcslen(cDigest);
		switch (i)
		{
		case 32:
			alg_id = MD5;
			break;
		case 40:
			alg_id = SHA1;
			break;
		case 64:
			alg_id = SHA256;
			break;
		case 96:
			alg_id = SHA384;
			break;
		case 128:
			alg_id = SHA512;
			break;
		default:
			alg_id = UNKNOWN_ALG;
			break;
		}
	}

	i = _tcslen(cDigest);
	if ((alg_id == MD5 && i != 32)
		|| (alg_id == SHA1 && i != 40)
		|| (alg_id == SHA256 && i != 64)
		|| (alg_id == SHA384 && i != 96)
		|| (alg_id == SHA512 && i != 128)
		|| alg_id == UNKNOWN_ALG)
	{
		delete[] cDigest;
		delete[] cFileName;
		return false;
	}

	zOut_FileNameInLine = cFileName;
	zOut_DigestInLine = cDigest;

	delete[] cDigest;
	delete[] cFileName;

	return true;
}

bool DigestFile(str& zIn_FileToCompute)
{
	str zDigestComputed;
	bool status = ComputeFileDigest(zIn_FileToCompute, zDigestComputed, g_option._digest_alg, g_option._binary);
	if (status)
	{
		outs.set_delimiter(g_option._delim);
		if (g_option._bsd_tag)
		{
			//BSD style (doesn't support '--text' mode):
			//MD5 (file) = 05b04f4921652d0bc7dbf0835ba89fe1
			outs().format(_T("%s (%s) = %s"),
				g_option._digest_alg_name.c_str(),
				zIn_FileToCompute.c_str(),
				zDigestComputed.c_str());
		}
		else
		{
			//GNU style:
			//05b04f4921652d0bc7dbf0835ba89fe1 *file
			//05b04f4921652d0bc7dbf0835ba89fe1  file
			outs().format(_T("%s %c%s"),
				zDigestComputed.c_str(),
				g_option._binary ? _T('*') : _T(' '),
				zIn_FileToCompute.c_str());
		}
	}
	return status;
}
bool DigestCheck(str& zIn_FileContainsDigestInfo)
{
	str zDigestComputed;
	DWORD nMisformattedLines = 0;
	DWORD nImproperlyFormattedLines = 0;
	DWORD nMismatchedChecksums = 0;
	DWORD nOpenOrReadFailures = 0;
	bool bProperlyFormattedLines = false;
	bool bMatchedChecksums = false;

	const int max_line_length = 1024;
	TCHAR* cLine = new TCHAR[max_line_length];

	bool is_binary = false;

	bool is_stdin = (zIn_FileContainsDigestInfo == _T("-"));

	FILE* f = NULL;

	if (is_stdin)
	{
		zIn_FileContainsDigestInfo = _T("standard input");
		f = stdin;
	}
	else
	{
		_tfopen_s(&f, zIn_FileContainsDigestInfo.c_str(), _T("r"));
		if (f == NULL)
		{
			errs().format(_T("%s: %s: no such file or directory"),
				g_option._program_name.c_str(), zIn_FileContainsDigestInfo.c_str());
			return false;
		}
	}

	DWORD nLine = 0;
	size_t nLineLen;
	std::vector<str> file;
	str zDigestInFile;
	AlgHash alg;
	do {
		++nLine;
		if (nLine == 0)
			errs(1).format(_T("%s: too many checksum lines"),
				zIn_FileContainsDigestInfo.c_str());
			

		if (NULL == _fgetts(cLine, max_line_length, f))
			break;

		nLineLen = _tcslen(cLine);
		//Ignore comment lines, which begin with a '#' character.
		if (cLine[0] == '#')
			continue;

		str zFileToCheck;
		bool bParseOk = ParseLine(cLine, zDigestInFile, zFileToCheck, is_binary, alg);
		if (!bParseOk || (bParseOk && (alg != g_option._digest_alg)))
		{
			++nMisformattedLines;
			++nImproperlyFormattedLines;
			errs(1, g_option._warn || g_option._status_only)
				.format(_T("%s: %lu: ill-formatted %s checksum line"),
					zIn_FileContainsDigestInfo.c_str(), nLine, g_option._digest_alg_name.c_str());
			
		}
		else
		{
			bool ok = false;

			bProperlyFormattedLines = true;
			zDigestComputed = _T("");//clear contents
			ok = ComputeFileDigest(zFileToCheck, zDigestComputed, g_option._digest_alg, is_binary);
			if (!ok)
			{
				++nOpenOrReadFailures;
				errs(0, g_option._status_only || g_option._ignore_missing)
					.format(_T("%s: open or read error"), zFileToCheck.c_str());
			}
			else
			{
				if (zDigestComputed != zDigestInFile)
					++nMismatchedChecksums;
				else
					bMatchedChecksums = true;

				outs(0, g_option._status_only).format(_T("%s: %s%c"),
					zFileToCheck.c_str(),
					(zDigestComputed != zDigestInFile) ? _T("FAILED") : ((!g_option._quiet) ? _T("OK") : _T("")),
					g_option._delim);
			}
		}
	} while (!feof(f) && !ferror(f));

	delete[] cLine;

	if ((!is_stdin && fclose(f) != 0) || (ferror(f)))
	{
		errs().format(_T("%s: read error"),
			zIn_FileContainsDigestInfo.c_str());
		return false;
	}

	if (!bProperlyFormattedLines)
	{
		//Warn if no tests are found.
		errs(1).format(_T("%s: no well-formatted %s checksum lines found"),
			zIn_FileContainsDigestInfo.c_str(), g_option._digest_alg_name.c_str());
	}
	else
	{
		if (!g_option._status_only)
		{
			errs(1, (nMisformattedLines == 0)).format(_T("WARNING: %lu: line(s) is ill-formatted"),
				nMisformattedLines);

			errs(1, (nOpenOrReadFailures == 0)).format(_T("WARNING: %lu: listed file(s) could not be read"),
				nOpenOrReadFailures);

			errs(1, (nMismatchedChecksums == 0)).format(_T("WARNING: %lu: computed checksum(s) did NOT match"),
				nMismatchedChecksums);

			errs(1, g_option._ignore_missing || bMatchedChecksums).format(_T("%s: no file was verified"),
				zIn_FileContainsDigestInfo.c_str());
		}
	}

	return (bProperlyFormattedLines
		&& bMatchedChecksums
		&& nMismatchedChecksums == 0
		&& nOpenOrReadFailures == 0
		&& (!g_option._strict || nImproperlyFormattedLines == 0));
}

int main(int argc, const TCHAR* argv[])
{
	g_option.InitMain(argc, argv);

	if (argc == 1)
	{
		errs().format(_T("%s: requires argument(s)."), g_option._program_name.c_str());
		g_option.DisposeInvalidOption(true);
	}
	
	option::definition optdefs[] = {
		{_T("--binary"), 'b', option::no_argument},
		{_T("--check"), 'c', option::no_argument},
		{_T("--ignore-missing"), -300, option::no_argument},
		{_T("--quiet"), -301, option::no_argument},
		{_T("--status"), -302, option::no_argument},
		{_T("--text"), 't', option::no_argument},
		{_T("--warn"), 'w', option::no_argument},
		{_T("--strict"), -303, option::no_argument},
		{_T("--tag"), -304, option::no_argument},
		{_T("--zero"), '0', option::no_argument},
		{_T("--help"), -305, option::no_argument},
		{_T("--version"), -306, option::no_argument},
		option::definition::nullopt() };

	option opt(argc, argv, optdefs);

	std::vector<str> files;
	while (!opt.is_end())
	{
		switch (opt.value())
		{
		case 'b':
			g_option._binary = true;
			g_option.SetBinaryFlag();
			break;
		case 'c':
			g_option._do_check = true;
			break;
		case 't':
			g_option._binary = false;
			break;
		case 'w':
			g_option._status_only = false;
			g_option._warn = true;
			g_option._quiet = false;
			break;
		case '0':
			g_option._delim = _T("");
			break;
		case -300:
			g_option._ignore_missing = true;
			break;
		case -301:
			g_option._status_only = false;
			g_option._warn = false;
			g_option._quiet = true;
			break;
		case -302:
			g_option._status_only = true;
			g_option._warn = false;
			g_option._quiet = false;
			break;
		case -303:
			g_option._strict = true;
			break;
		case -304:
			g_option._bsd_tag = true;
			g_option._binary = true;
			break;
		case -305:
			Usage(EXIT_SUCCESS);
			break;
		case -306:
			Version();
			break;
		default:
			if (opt.kind() == option::operand)
			{
				ParseFileName(files, opt.optname());
				break;
			}
			else
				g_option.DisposeInvalidOption(opt.has_error());
		}
		opt.to_next();
	}

	g_option.DisposeOptionConflict();

	struct RUN_T
	{
		int status;
		RUN_T() : status(EXIT_SUCCESS) {}

		void operator()(str& zFile)
		{
			if (g_option._do_check)
			{
				if (!DigestCheck(zFile))
					status = EXIT_FAILURE;
			}
			else
			{
				if (!DigestFile(zFile))
					status = EXIT_FAILURE;
			}
		}
	} _run;
	std::for_each(files.begin(), files.end(), _run);
	outs.print();
	errs.print();
	return _run.status;
}
