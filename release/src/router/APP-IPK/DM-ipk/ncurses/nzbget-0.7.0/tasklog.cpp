/*
 *  This file is part of nzbget
 *
 *  Copyright (C) 2007-2009 Andrei Prygounkov <hugbug@users.sourceforge.net>
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Revision: 1.1.1.1 $
 * $Date: 2011/03/21 01:46:15 $
 *
 */

#include <stdio.h>
#include "tasklog.h"


Tasklog::Tasklog()
{

}

/*int Tasklog::GetLogID()
{
    return m_logID;
}

void Tasklog::SetLogID(int id)
{
    m_logID = id;
}

void Tasklog::log_init(FileInfo* pFileInfo)
{
    char path[256];
    memset(path, 0, sizeof(path));
    sprintf(path, "./.logs/nzb_%d", m_logID);

    FILE *fp = fopen(path, "w");
    if(fp)
    {
        // do
        log_s.id = pFileInfo->GetID();
        time(&log_s.begin_t);
        log_s.download_type = NZB;
        strcpy(log_s.filename, pFileInfo->GetFilename());
        log_s.filesize = pFileInfo->GetSize();
        log_s.status = S_PROCESSING;

        fwrite(&log_s, 1, LOG_SIZE, fp);
    }

    fclose(fp);
}

void Tasklog::updateLog(FileInfo* pFileInfo)
{
}

int Tasklog::GetTaskID()
{
    return log_s.id;
}

int Tasklog::IsCompleted()
{
    return log_s.status;
}
*/
