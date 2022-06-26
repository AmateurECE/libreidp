///////////////////////////////////////////////////////////////////////////////
// NAME:            configuration.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     Logic and structures for handling configuration.
//
// CREATED:         06/26/2022
//
// LAST EDITED:     06/26/2022
//
// Copyright 2022, Ethan D. Twardy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
////

use serde::Deserialize;
use crate::cache::CacheConfiguration;

// Configuration for the LDAP-side
#[derive(Default, Deserialize)]
pub struct LdapConfiguration {
    pub uri: String,
    pub base: String,
}

// Configuration for the HTTP-side
#[derive(Default, Deserialize)]
pub struct HttpConfiguration {
    pub address: String,
}

// Serializable configuration object
#[derive(Default, Deserialize)]
pub struct IdPConfiguration {
    pub ldap: LdapConfiguration,
    pub http: HttpConfiguration,
    pub cache: CacheConfiguration,
}

///////////////////////////////////////////////////////////////////////////////
