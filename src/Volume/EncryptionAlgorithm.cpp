/*
 Derived from source code of TrueCrypt 7.1a, which is
 Copyright (c) 2008-2012 TrueCrypt Developers Association and which is governed
 by the TrueCrypt License 3.0.

 Modifications and additions to the original source code (contained in this file)
 and all other portions of this file are Copyright (c) 2013-2025 AM Crypto
 and are governed by the Apache License 2.0 the full text of which is
 contained in the file License.txt included in VeraCrypt binary and source
 code distribution packages.
*/

#include "EncryptionAlgorithm.h"
#include "EncryptionModeXTS.h"
#ifdef WOLFCRYPT_BACKEND
#include "EncryptionModeWolfCryptXTS.h"
#endif

namespace VeraCrypt
{
	EncryptionAlgorithm::EncryptionAlgorithm () : Deprecated (false)
	{
	}

	EncryptionAlgorithm::~EncryptionAlgorithm ()
	{
	}

	void EncryptionAlgorithm::Decrypt (uint8 *data, uint64 length) const
	{
		if_debug (ValidateState ());
		Mode->Decrypt (data, length);
	}

	void EncryptionAlgorithm::Decrypt (const BufferPtr &data) const
	{
		Decrypt (data, data.Size());
	}

	void EncryptionAlgorithm::DecryptSectors (uint8 *data, uint64 sectorIndex, uint64 sectorCount, size_t sectorSize) const
	{
		if_debug (ValidateState());
		Mode->DecryptSectors (data, sectorIndex, sectorCount, sectorSize);
	}

	void EncryptionAlgorithm::Encrypt (uint8 *data, uint64 length) const
	{
		if_debug (ValidateState());
		Mode->Encrypt (data, length);
	}

	void EncryptionAlgorithm::Encrypt (const BufferPtr &data) const
	{
		Encrypt (data, data.Size());
	}

	void EncryptionAlgorithm::EncryptSectors (uint8 *data, uint64 sectorIndex, uint64 sectorCount, size_t sectorSize) const
	{
		if_debug (ValidateState ());
		Mode->EncryptSectors (data, sectorIndex, sectorCount, sectorSize);
	}

	EncryptionAlgorithmList EncryptionAlgorithm::GetAvailableAlgorithms ()
	{
		EncryptionAlgorithmList l;

		l.push_back (shared_ptr <EncryptionAlgorithm> (new AES ()));
        #ifndef WOLFCRYPT_BACKEND
		l.push_back (shared_ptr <EncryptionAlgorithm> (new Serpent ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new Twofish ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new Camellia ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new Kuznyechik ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new SM4 ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new AESTwofish ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new AESTwofishSerpent ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new CamelliaKuznyechik ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new CamelliaSerpent ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new KuznyechikAES ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new KuznyechikSerpentCamellia ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new KuznyechikTwofish ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new SerpentAES ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new SerpentTwofishAES ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new TwofishSerpent ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new KuznyechikSM4 ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new SerpentSM4 ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new SM4Twofish ()));
		l.push_back (shared_ptr <EncryptionAlgorithm> (new TwofishSerpentSM4 ()));
        #endif
		return l;
	}

	size_t EncryptionAlgorithm::GetLargestKeySize (const EncryptionAlgorithmList &algorithms)
	{
		size_t largestKeySize = 0;

		foreach_ref (const EncryptionAlgorithm &ea, algorithms)
		{
			if (ea.GetKeySize() > largestKeySize)
				largestKeySize = ea.GetKeySize();
		}

		return largestKeySize;
	}

	size_t EncryptionAlgorithm::GetKeySize () const
	{
		if (Ciphers.size() < 1)
			throw NotInitialized (SRC_POS);

		size_t keySize = 0;

		foreach_ref (const Cipher &c, Ciphers)
			keySize += c.GetKeySize();

		return keySize;
	}

	size_t EncryptionAlgorithm::GetMaxBlockSize () const
	{
		size_t blockSize = 0;

		foreach_ref (const Cipher &c, Ciphers)
			if (c.GetBlockSize() > blockSize)
				blockSize = c.GetBlockSize();

		return blockSize;
	}

	size_t EncryptionAlgorithm::GetMinBlockSize () const
	{
		size_t blockSize = 0;

		foreach_ref (const Cipher &c, Ciphers)
			if (blockSize == 0 || c.GetBlockSize() < blockSize)
				blockSize = c.GetBlockSize();

		return blockSize;
	}

	shared_ptr <EncryptionMode> EncryptionAlgorithm::GetMode () const
	{
		if (Mode.get() == nullptr)
			throw NotInitialized (SRC_POS);

		return Mode;
	}

	wstring EncryptionAlgorithm::GetName (bool forGuiDisplay) const
	{
		if (Ciphers.size() < 1)
			throw NotInitialized (SRC_POS);

		wstring name;

		int depth = 0;
		foreach_reverse_ref (const Cipher &c, Ciphers)
		{
			if (name.empty())
				name = c.GetName();
			else
			{
				depth++;
				if (forGuiDisplay)
					name += wstring (L"(");
				else
					name += wstring (L"-");
				name += c.GetName();
			}
		}

		if (forGuiDisplay && depth)
		{
			for (int i = 0; i < depth; i++)
				name += wstring(L")");
		}

		return name;
	}

	bool EncryptionAlgorithm::IsModeSupported (const EncryptionMode &mode) const
	{
		bool supported = false;

		foreach_ref (const EncryptionMode &em, SupportedModes)
		{
			if (typeid (mode) == typeid (em))
			{
				supported = true;
				break;
			}
		}

		return supported;
	}


	bool EncryptionAlgorithm::IsModeSupported (const shared_ptr <EncryptionMode> mode) const
	{
		return IsModeSupported (*mode);
	}

	void EncryptionAlgorithm::SetMode (shared_ptr <EncryptionMode> mode)
	{
		if (!IsModeSupported (*mode))
			throw ParameterIncorrect (SRC_POS);

		mode->SetCiphers (Ciphers);
		Mode = mode;
	}

	void EncryptionAlgorithm::SetKey (const ConstBufferPtr &key)
	{
		if (Ciphers.size() < 1)
			throw NotInitialized (SRC_POS);

		if (GetKeySize() != key.Size())
			throw ParameterIncorrect (SRC_POS);

		size_t keyOffset = 0;
		foreach_ref (Cipher &c, Ciphers)
		{
			c.SetKey (key.GetRange (keyOffset, c.GetKeySize()));
			keyOffset += c.GetKeySize();
		}
	}

    #ifdef WOLFCRYPT_BACKEND
        void EncryptionAlgorithm::SetKeyXTS (const ConstBufferPtr &key)
	{
		if (Ciphers.size() < 1)
			throw NotInitialized (SRC_POS);

		if (GetKeySize() != key.Size())
			throw ParameterIncorrect (SRC_POS);

		size_t keyOffset = 0;
		foreach_ref (Cipher &c, Ciphers)
		{
			c.SetKeyXTS (key.GetRange (keyOffset, c.GetKeySize()));
			keyOffset += c.GetKeySize();
		}
	}
    #endif

        void EncryptionAlgorithm::ValidateState () const
	{
		if (Ciphers.size() < 1 || Mode.get() == nullptr)
			throw NotInitialized (SRC_POS);
	}

	// AES
	AES::AES ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherAES()));

            #ifdef WOLFCRYPT_BACKEND
                SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeWolfCryptXTS ()));
            #else
		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
            #endif
        }

#ifndef WOLFCRYPT_BACKEND
	// AES-Twofish
	AESTwofish::AESTwofish ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherTwofish ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherAES ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}

	// AES-Twofish-Serpent
	AESTwofishSerpent::AESTwofishSerpent ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherSerpent ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherTwofish ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherAES ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}

	// Serpent
	Serpent::Serpent ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherSerpent()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}

	// Serpent-AES
	SerpentAES::SerpentAES ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherAES ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherSerpent ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}

	// Twofish
	Twofish::Twofish ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherTwofish()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}

	// Twofish-Serpent
	TwofishSerpent::TwofishSerpent ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherSerpent ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherTwofish ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}

	// Serpent-Twofish-AES
	SerpentTwofishAES::SerpentTwofishAES ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherAES ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherTwofish ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherSerpent ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}
	
	// Camellia
	Camellia::Camellia ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherCamellia()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}

	// Kuznyechik
	Kuznyechik::Kuznyechik ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherKuznyechik()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}
	
	// Kuznyechik-Twofish
	KuznyechikTwofish::KuznyechikTwofish ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherTwofish ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherKuznyechik ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}
	
	// Kuznyechik-AES
	KuznyechikAES::KuznyechikAES ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherAES ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherKuznyechik ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}
	
	// Kuznyechik-Serpent-Camellia
	KuznyechikSerpentCamellia::KuznyechikSerpentCamellia ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherCamellia ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherSerpent ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherKuznyechik ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}
	
	// Camellia-Kuznyechik
	CamelliaKuznyechik::CamelliaKuznyechik ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherKuznyechik ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherCamellia ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}
	
	// Camellia-Serpent
	CamelliaSerpent::CamelliaSerpent ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherSerpent ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherCamellia ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}

	// SM4
	SM4::SM4 ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherSM4()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}
	
	// Kuznyechik-SM4
	KuznyechikSM4::KuznyechikSM4 ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherSM4 ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherKuznyechik ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}

	// Serpent-SM4
	SerpentSM4::SerpentSM4 ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherSM4 ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherSerpent ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}

    // SM4-Twofish
    SM4Twofish::SM4Twofish ()
	{
        Ciphers.push_back (shared_ptr <Cipher> (new CipherTwofish ()));
		Ciphers.push_back (shared_ptr <Cipher> (new CipherSM4 ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}

    // Twofish-Serpent-SM4
    TwofishSerpentSM4::TwofishSerpentSM4 ()
	{
		Ciphers.push_back (shared_ptr <Cipher> (new CipherSM4 ()));
        Ciphers.push_back (shared_ptr <Cipher> (new CipherSerpent ()));
        Ciphers.push_back (shared_ptr <Cipher> (new CipherTwofish ()));

		SupportedModes.push_back (shared_ptr <EncryptionMode> (new EncryptionModeXTS ()));
	}
#endif
}
