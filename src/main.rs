///////////////////////////////////////////////////////////////////////////////
// NAME:            main.rs
//
// AUTHOR:          Ethan D. Twardy <ethan.twardy@gmail.com>
//
// DESCRIPTION:     libreidp entrypoint.
//
// CREATED:         02/23/2022
//
// LAST EDITED:     06/12/2022
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

use std::error::Error;
use std::fs;
use axum::{routing::get, Router};
use clap::Parser;
use serde::Deserialize;
use tower_http::trace::TraceLayer;
use tracing::{event, Level};

// async fn query() -> Result<Body> {
//     println!("Opening connection");
//     let hostname = "ldap://edtwardy-webservices_openldap_1";
//     let (conn, mut ldap) = LdapConnAsync::new(hostname).await?;
//     ldap3::drive!(conn);
//     let (rs, _res) = ldap.search(
//         "ou=people,dc=edtwardy,dc=hopto,dc=org",
//         Scope::Subtree,
//         "(objectClass=*)",
//         vec!["*"]
//     ).await?.success()?;

//     ldap.unbind().await?;
//     Ok(Body::from(
//         rs.into_iter()
//             .map(|s| format!("{:?}\n", SearchEntry::construct(s)))
//             .collect::<String>()))
// }

#[derive(Parser, Debug)]
#[clap(author, version, about = None, long_about = None)]
struct Args {
    #[clap(short, long)]
    config_file: String,
}

#[derive(Default, Deserialize)]
struct Configuration {
    listen_address: String,
    ldap_uri: String,
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn Error>> {
    // Configure tower-http to trace requests/responses
    std::env::set_var("RUST_LOG", "tower_http=trace");
    tracing_subscriber::fmt::init();

    let args = Args::parse();
    event!(Level::INFO, "Opening configuration file {}", args.config_file);
    let configuration: Configuration = serde_yaml::from_reader(
        fs::File::open(args.config_file)?)?;
    let app = Router::new()
        .route("/", get(|| async { "Hello, World!" }))
        .layer(TraceLayer::new_for_http())
        ;

    let address = configuration.listen_address.parse()?;
    event!(Level::INFO, "Starting server on {}", &address);
    axum::Server::bind(&address)
        .serve(app.into_make_service())
        .await?;
    Ok(())
}

///////////////////////////////////////////////////////////////////////////////
