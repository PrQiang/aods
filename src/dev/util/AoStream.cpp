/*
 * Copyright (c) eryue, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
*/
#include "AoStream.h"
CAoStream::CAoStream(){}
CAoStream::~CAoStream(){}
int CAoStream::Seek( ao_size_t , ao_size_t  ){	return 0;}
ao_size_t CAoStream::Tell(){	return 0;}
ao_size_t CAoStream::Read( char* , ao_size_t  ){	return 0;}
ao_size_t CAoStream::Write( const char* , ao_size_t  ){	return 0;}
void CAoStream::Close(){}