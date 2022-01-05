// 
// -----------------------------------------------------------------------------
// errorcodes.h
// -----------------------------------------------------------------------------
//
// Internal error codes for Preview
//
// Copyright © 2021 Tyler Wayne
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef ERRORCODES_INCLUDED
#define ERRORCODES_INCLUDED

#define E_OK 0

#define E_DTA_FILE_ERROR 1
#define E_DTA_COL_OOB 2
#define E_DTA_ROW_OOB 3
#define E_DTA_MISSING_FIELD 4
#define E_DTA_RESOURCE_ERROR 5
#define E_DTA_PARSE_ERROR 6
#define E_DTA_BAD_INPUT 7
#define E_DTA_MAX_ROWS 8
#define E_DTA_EOF 9

// TODO: add frame error codes

#endif // ERRORCODES_INCLUDED
