/******************************************************************************
 *  utils.h
 *
 *  This file is part of Public Scripts
 *  Copyright (C) 2005-2011 Tom N Harris <telliamed@whoopdedo.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/
#include <lg/types.h>
#include <lg/defs.h>

/**
 * FixupScriptParamsHack
 *
 * If the string ``pszData`` begins with ''#'', then the object with
 * the name following the ''#'' is read and its ``Editor\Design Note``
 * property is returned.
 * A string that begins with ''.'' is an abbreviated name.
 * A string that begins with ''!'' has the string ''Stimulus'' added to it.
 * Otherwise, a copy of ``pszData`` is returned.
 * The return value is allocated using ``g_pMalloc``.
 */
char* FixupScriptParamsHack(const char* pszData);

/**
 * StringToMultiParm
 *
 * Parse the string ``psz`` and assign the interpreted value to ``mp``.
 * The string format is one of the characters ''i'', ''f'', ''s'', or ''v''
 * followed by the data. If there is no character code, then
 * the string is first converted to an integer, or if the entire
 * string cannot be converted, it is copied as a string.
 * The ''i'' format is an integer number. The ''f'' format is a
 * floating-point number. The ''v'' format is a 3-element vector
 * that is written as $$%f, %f, %f$$. The ''s'' format just copies
 * the data string.
 */
void StringToMultiParm(cMultiParm &mp, const char* psz);

/**
 * CalculateCurve
 *
 * Returns a value x = a + (c(f) * (b - a)) where c is a function
 * that returns a positive value less than 1.
 * The possible functions are:
 *   1 - square: c(f) = f * f
 *   2 - square root: c(f) = sqrt(f)
 *   3 - log: c(f) = 1 + log(f * 0.9 + 0.1)
 *   4 - 10^: c(f) = (10^f - 1) * 0.11111
 *   5 - ln: c(f) = 1 + ln(f * (1 - 1/e) + 1/e)
 *   6 - e^: c(f) = (e^f - 1) * 1/(e - 1)
 * The second version of the function will read the function type
 * from the ``DesignNote`` parameter $$curve$$ on the object.
 */
double CalculateCurve(int c, double f, double a, double b);
double CalculateCurve(double f, double a, double b, object iObj);

/**
 * SoundService compatibility functions
 *
 * Calls the appropriate ``ISoundScrSrv`` method.
 */
bool PlaySound(object host, const cScrStr& name, const cScrVec& location, eSoundSpecial flags=kSoundNormal, eSoundNetwork net=kSoundNetNormal);
bool PlaySound(object host, const cScrStr& name, object obj, eSoundSpecial flags=kSoundNormal, eSoundNetwork net=kSoundNetNormal);
bool PlaySound(object host, const cScrStr& name, eSoundSpecial flags=kSoundNormal, eSoundNetwork net=kSoundNetNormal);
bool PlayAmbient(object host, const cScrStr& name, eSoundSpecial flags=kSoundNormal, eSoundNetwork net=kSoundNetNormal);
bool PlaySchema(object host, int schema, const cScrVec& location, eSoundNetwork net=kSoundNetNormal);
bool PlaySchema(object host, object schema, object obj, eSoundNetwork net=kSoundNetNormal);
bool PlaySchema(object host, object schema, eSoundNetwork net=kSoundNetNormal);
bool PlaySchemaAmbient(object host, object schema, eSoundNetwork net=kSoundNetNormal);
bool PlayEnvSchema(object host,const cScrStr& name, object obj, object subj, eEnvSoundLoc loc, eSoundNetwork net=kSoundNetNormal);
bool PlayVoiceOver(object host, object schema);
int HaltSound(object host ,const cScrStr& name, object obj);
bool HaltSchema(object host,const cScrStr& name, object obj);
long HaltSpeech(object obj);
bool PreLoad(const cScrStr& name);

/**
 * ARStimulate compatibility macro
 *
 * Use as ''pActReactScrSrv->ARStimulate(??obj??,??stim??,??val??,??src??);''
 */
#if (_DARKGAME == 1)
#define ARStimulate(__obj,__stim,__intensity,__source)	Stimulate((__obj),(__stim),(__intensity))
#else
#define ARStimulate(__obj,__stim,__intensity,__source)	Stimulate((__obj),(__stim),(__intensity),(__source))
#endif

/**
 * GetBookText
 *
 * Fetch the contents of a book or use message.
 * In Thief, gets the name of a book from ``Book\Text`` and reads the book file.
 * In SShock2, gets the name of a string from ``Script\Use Message`` and reads the text from ``UseMsg.str``.
 */
cAnsiStr GetBookText(object iObj);

/**
 * strniscmp
 *
 * Compare strings case-insensitively and considering only alphanumeric characters.
 * End the comparison after reading ??len?? bytes (including spaces) from ??str1??.
 */
int strnalnumcmp(const char* str1, const char* str2, size_t len);
