use flate2::read::GzDecoder;
use std::{
    env::args,
    fs::{create_dir, remove_file, File},
    io::{copy, BufReader},
    path::{Path, PathBuf},
};
use tar::Archive;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let node_target = args().nth(1).expect("Missing node target");

    let to = format!("node-{}-v8-source", node_target);
    let to = Path::new(&to);

    let resp = attohttpc::get(format!(
        "http://nodejs.org/dist/{0}/node-{0}.tar.gz",
        node_target
    ))
    .send()?;

    let mut extractor = Archive::new(GzDecoder::new(resp));
    for file in extractor.entries()? {
        let file = file?;
        let file_path = file.path()?.components().skip(1).collect::<PathBuf>();
        if !file_path.starts_with("deps/v8") {
            continue;
        }

        let outpath = to.join(file_path.strip_prefix("deps/v8")?);

        if file.header().entry_type().is_dir() {
            create_dir(outpath)?;
        } else {
            let mut outfile = File::create(outpath)?;
            copy(&mut BufReader::new(file), &mut outfile)?;
        }
    }

    Ok(())
}
