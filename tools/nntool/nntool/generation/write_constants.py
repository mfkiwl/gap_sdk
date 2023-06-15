# Copyright (C) 2020  GreenWaves Technologies, SAS

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.

# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import os


def write_constants(global_recs, tensor_directory=None):
    if tensor_directory is None:
        tensor_directory = "."
    elif tensor_directory != ".":
        os.makedirs(tensor_directory, mode=0o750, exist_ok=True)
    for global_rec in global_recs:
        if global_rec.const_info is None:
            continue
        const_info = global_rec.const_info
        with open(os.path.join(tensor_directory, const_info.file_name), 'wb') as t_fp:
            const_info.contents.astype(const_info.qtype.dtype, order='C', casting='unsafe', copy=True)\
                .tofile(t_fp)
