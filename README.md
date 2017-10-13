# Islandora Datastream Exporter

## Introduction

This module provides a Drush script that can be used to bulk export datastreams
given a Solr query.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Tuque](https://github.com/islandora/tuque)
* [Islandora Solr Search](https://github.com/islandora/islandora_solr_search)

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Usage
Output of ```drush islandora_datastream_export --help:```

```
Exports a specified datastream from all objects given a fielded Solr query.

Examples:
 drush -u 1 islandora_datastream_export  Exporting datastream from object.
 --export_target=/tmp --query=PID:\"islandora:9\" --dsid=DC

Options:
 --dsid                                    The datastream id of to be exported datastream. Required.
 --query                                   The Solr query to be ran. Required.
 --export_target                                  The directory to export the datastreams to. Required.
  ```

It's to be noted that when specifying a value that some values will need to be
escaped as the value is passed directly to Solr. An example of this is for the
PID field where `islandora:test` will not work, while `"islandora:test"` or
`islandora\:test` will. For queries taking advantage of Lucene syntax all parts
of the query string must be provided as escaped. Boolean logic is allowed.

Finally the user option (-u) needs to be specified or errors could be
encountered when attempting to write the contents of the datastream to a file.

## Maintainers/Sponsors

Current maintainers:

* [William Panting](https://github.com/willtp87)
* [discoverygarden](https://github.com/discoverygarden)

This project has been sponsored by:

* [University of Saskatchewan](www.usask.ca)
The University of Saskatchewan is a Canadian public research university, founded
in 1907, and located on the east side of the South Saskatchewan River in
Saskatoon, Saskatchewan, Canada.

## Development

If you would like to contribute to this module, please check out our helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the Islandora.ca site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
