<?php

/**
 * @file
 * Functionality for the Solr query backend.
 */

/**
 * Callback to validate the Solr query.
 */
function islandora_datastream_exporter_solr_query_validate($query) {
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery($query);
  $qp->solrLimit = 1;
  $qp->executeQuery(FALSE);
  if (empty($qp->islandoraSolrResult)) {
    return drush_set_error('Invalid Solr query', dt('The specified query !query is not valid. Please ensure you have a correct and escaped query.', array('!query' => $query)));
  }
  elseif ($qp->islandoraSolrResult['response']['numFound'] === 0) {
    return drush_set_error('No result Solr query', dt('The specified query !query returned no results.', array('!query' => $query)));
  }
}

/**
 * Callback to provide PIDs from the Solr query.
 */
function islandora_datastream_exporter_solr_query_pid_source($query, $max, &$context) {
  $sandbox = &$context['sandbox'];
  $qp = new IslandoraSolrQueryProcessor();
  $qp->buildQuery($query);
  if (!isset($sandbox['offset'])) {
    $qp->executeQuery(FALSE);
    $sandbox['total'] = $qp->islandoraSolrResult['response']['numFound'];
    $sandbox['offset'] = 0;
  }
  $context['message'] = t('Processing results @start to @end.', array(
    '@start' => $sandbox['offset'] + 1,
    '@end' => min($sandbox['offset'] + $max, $sandbox['total']),
  ));
  $qp->solrLimit = $max;
  $qp->solrStart = $sandbox['offset'];
  $qp->executeQuery();
  $results = $qp->islandoraSolrResult['response']['objects'];
  $sandbox['offset'] += $max;
  $get_pid = function ($doc) {
    return $doc['PID'];
  };
  $context['finished'] = $sandbox['offset'] / $sandbox['total'];
  return array_map($get_pid, $results);
}
