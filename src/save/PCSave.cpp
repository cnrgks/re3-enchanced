#define WITHWINDOWS
#include "common.h"
#include "crossplatform.h"

#include "FileMgr.h"
#include "Font.h"
#ifdef MORE_LANGUAGES
#include "Game.h"
#endif
#include "GenericGameStorage.h"
#include "Messages.h"
#include "PCSave.h"
#include "Enhanced.h"
#include "PlayerPed.h"
#include "PlayerInfo.h"
#include "Text.h"

const char* _psGetUserFilesFolder();

C_PcSave PcSaveHelper;

void
C_PcSave::SetSaveDirectory(const char *path)
{
	sprintf(DefaultPCSaveFileName, "%s\\%s", path, "GTA3sf");
}

bool
C_PcSave::DeleteSlot(int32 slot)
{
#ifdef FIX_BUGS
	char FileName[MAX_PATH];
#else
	char FileName[200];
#endif

	PcSaveHelper.nErrorCode = SAVESTATUS_SUCCESSFUL;
	sprintf(FileName, "%s%i.b", DefaultPCSaveFileName, slot + 1);
	DeleteFile(FileName);
	SlotSaveDate[slot][0] = '\0';
	return true;
}

bool
C_PcSave::SaveSlot(int32 slot, bool autosave)
{
    CPlayerPed *player = FindPlayerPed();
    const float previousHealth = player ? player->m_fHealth : 0.0f;
    bool heal = Enhanced::HealAfterSave && player && slot >= 0 && slot < SLOT_COUNT;
#ifdef MISSION_REPLAY
    heal = heal && !IsQuickSave;
#endif
	MakeValidSaveName(slot);
	PcSaveHelper.nErrorCode = SAVESTATUS_SUCCESSFUL;
	_psGetUserFilesFolder();
	char temporaryName[sizeof(ValidSaveName) + sizeof(".tmp")];
	snprintf(temporaryName, sizeof(temporaryName), "%s.tmp", ValidSaveName);
	const int file = CFileMgr::OpenFile(autosave ? temporaryName : ValidSaveName, "wb");
	if (file == 0) {
		nErrorCode = SAVESTATUS_ERR_SAVE_CREATE;
		return false;
	}

	if (!autosave
#ifdef MISSION_REPLAY
	    && !IsQuickSave
#endif
	)
		DoGameSpecificStuffBeforeSave();
	if (heal) player->m_fHealth = Max(previousHealth, 100.0f);
	bool saved = GenericSave(file);
	if (CFileMgr::CloseFile(file) != 0) {
		nErrorCode = SAVESTATUS_ERR_SAVE_CLOSE;
		saved = false;
	}
	if (autosave && saved && !ValidateSaveFile(temporaryName)) {
		nErrorCode = SAVESTATUS_ERR_SAVE_WRITE;
		saved = false;
	}
	if (autosave && saved) {
		// Preserve the old slot until the replacement is fully written and closed.
#ifdef _WIN32
		saved = MoveFileExA(temporaryName, ValidSaveName, MOVEFILE_REPLACE_EXISTING) != 0;
#else
		char *sourcePath = casepath(temporaryName);
		char *targetPath = casepath(ValidSaveName);
		saved = rename(sourcePath ? sourcePath : temporaryName, targetPath ? targetPath : ValidSaveName) == 0;
		free(sourcePath);
		free(targetPath);
#endif
		if (!saved)
			nErrorCode = SAVESTATUS_ERR_SAVE_WRITE;
	}
	if (!saved) {
		if (autosave) {
#ifdef _WIN32
			DeleteFile(temporaryName);
#else
			char *path = casepath(temporaryName);
			unlink(path ? path : temporaryName);
			free(path);
#endif
		}
		if (heal) player->m_fHealth = previousHealth;
	}
	return saved;
}

bool
C_PcSave::PcClassSaveRoutine(int32 file, uint8 *data, uint32 size)
{
	CFileMgr::Write(file, (const char*)&size, sizeof(size));
	if (CFileMgr::GetErrorReadWrite(file)) {
		nErrorCode = SAVESTATUS_ERR_SAVE_WRITE;
		strncpy(SaveFileNameJustSaved, ValidSaveName, sizeof(ValidSaveName) - 1);
		return false;
	}

	CFileMgr::Write(file, (const char*)data, align4bytes(size));
	CheckSum += (uint8) size;
	CheckSum += (uint8) (size >> 8);
	CheckSum += (uint8) (size >> 16);
	CheckSum += (uint8) (size >> 24);
	for (int i = 0; i < align4bytes(size); i++) {
		CheckSum += *data++;
	}
	if (CFileMgr::GetErrorReadWrite(file)) {
		nErrorCode = SAVESTATUS_ERR_SAVE_WRITE;
		strncpy(SaveFileNameJustSaved, ValidSaveName, sizeof(ValidSaveName) - 1);
		return false;
	}

	return true;
}

void
C_PcSave::PopulateSlotInfo()
{
	for (int i = 0; i < SLOT_COUNT; i++) {
		Slots[i + 1] = SLOT_EMPTY;
		SlotFileName[i][0] = '\0';
		SlotSaveDate[i][0] = '\0';
	}
	for (int i = 0; i < SLOT_COUNT; i++) {
#ifdef FIX_BUGS
		char savename[MAX_PATH];
#else
		char savename[52];
#endif
		struct {
			int size;
			wchar FileName[24];
			SYSTEMTIME SaveDateTime;
		} header;
		sprintf(savename, "%s%i%s", DefaultPCSaveFileName, i + 1, ".b");
		int file = CFileMgr::OpenFile(savename, "rb");
		if (file != 0) {
			const size_t headerSize = CFileMgr::Read(file, (char*)&header, sizeof(header));
			if (headerSize != sizeof(header)) {
				CMessages::InsertNumberInString(TheText.Get("FEC_SLC"), i + 1, -1, -1, -1, -1, -1, SlotFileName[i]);
				Slots[i + 1] = SLOT_CORRUPTED;
			} else if (strncmp((char*)&header, TopLineEmptyFile, sizeof(TopLineEmptyFile)-1) != 0) {
				Slots[i + 1] = SLOT_OK;
				memcpy(SlotFileName[i], &header.FileName, sizeof(header.FileName));
				
				SlotFileName[i][24] = '\0';
			}
			CFileMgr::CloseFile(file);
		}
		if (Slots[i + 1] == SLOT_OK) {
			if (CheckDataNotCorrupt(i, savename)) {
#ifdef FIX_INCOMPATIBLE_SAVES
				if (!FixSave(i, GetSaveType(savename))) {
					CMessages::InsertNumberInString(TheText.Get("FEC_SLC"), i + 1, -1, -1, -1, -1, -1, SlotFileName[i]);
					Slots[i + 1] = SLOT_CORRUPTED;
					continue;
				}
#endif
				SYSTEMTIME st;
				memcpy(&st, &header.SaveDateTime, sizeof(SYSTEMTIME));
				const char *month;
				switch (st.wMonth)
				{
				case 1: month = "JAN"; break;
				case 2: month = "FEB"; break;
				case 3: month = "MAR"; break;
				case 4: month = "APR"; break;
				case 5: month = "MAY"; break;
				case 6: month = "JUN"; break;
				case 7: month = "JUL"; break;
				case 8: month = "AUG"; break;
				case 9: month = "SEP"; break;
				case 10: month = "OCT"; break;
				case 11: month = "NOV"; break;
				case 12: month = "DEC"; break;
				default:
					CMessages::InsertNumberInString(TheText.Get("FEC_SLC"), i + 1, -1, -1, -1, -1, -1, SlotFileName[i]);
					Slots[i + 1] = SLOT_CORRUPTED;
					continue;
				}
				char date[70];
#ifdef MORE_LANGUAGES
				if (CGame::japaneseGame)
					sprintf(date, "%02d %02d %04d %02d:%02d:%02d", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
				else
#endif // MORE_LANGUAGES
					sprintf(date, "%02d %s %04d %02d:%02d:%02d", st.wDay, UnicodeToAsciiForSaveLoad(TheText.Get(month)), st.wYear, st.wHour, st.wMinute, st.wSecond);
				AsciiToUnicode(date, SlotSaveDate[i]);

			} else {
				CMessages::InsertNumberInString(TheText.Get("FEC_SLC"), i + 1, -1, -1, -1, -1, -1, SlotFileName[i]);
				Slots[i + 1] = SLOT_CORRUPTED;
			}
		}
	}
}
