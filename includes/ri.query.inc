<?php

/**
 * @file
 * Functionality for the RI query backend.
 */

/**
 * Callback to validate the RI query.
 */
function islandora_datastream_exporter_ri_query_validate($query) {
  if (!is_readable($query)) {
    return drush_set_error('Cannot read file', dt('Could not read the file specified at !path', array('!path' => $query)));
  }
  $tuque = islandora_get_tuque_connection();
  try {
    $results = $tuque->repository->ri->sparqlQuery(_islandora_datastream_exporter_ri_query_offset_replace(file_get_contents($query), 0), 1);
    if (empty($results)) {
      return drush_set_error('No result RI query', dt('The specified query at the given location returned no results.'));
    }
  }
  catch (RepositoryException $e) {
    return drush_set_error('Invalid request', dt('An error occurred attempting to perform the query provided: !e', array('!e' => $e->getMessage())));
  }
}

/**
 * Callback to provide PIDs from the RI query.
 */
function islandora_datastream_exporter_ri_query_pid_source($query, $max, &$context) {
  $tuque = islandora_get_tuque_connection();
  $sandbox = &$context['sandbox'];
  if (!isset($sandbox['offset'])) {
    $sandbox['query'] = file_get_contents($query);
    $sandbox['total'] = $tuque->repository->ri->countQuery(_islandora_datastream_exporter_ri_query_offset_replace($sandbox['query'], 0), 'sparql');
    $sandbox['offset'] = 0;
  }
  $results = $tuque->repository->ri->sparqlQuery(_islandora_datastream_exporter_ri_query_offset_replace($sandbox['query'], $sandbox['offset']), $max);
  $context['message'] = t('Processing results @start to @end', array(
    '@start' => $sandbox['offset'] + 1,
    '@end' => min($sandbox['offset'] + $max, $sandbox['total']),
  ));
  $sandbox['offset'] += count($results);
  $get_pid = function ($result) {
    return $result['pid']['value'];
  };
  $context['finished'] = $sandbox['offset'] / $sandbox['total'];
  return array_map($get_pid, $results);
}

/**
 * Helper to swap the %offset% for a proper query.
 */
function _islandora_datastream_exporter_ri_query_offset_replace($query, $offset) {
  return str_replace('%offset%', $offset, $query);
}
