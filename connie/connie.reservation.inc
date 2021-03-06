<?php

/**
 * @file
 * Ding reservation provider example.
 */

/**
 *
 */
function connie_reservation_pickup_branches($account) {
  // Throw exception if we're not logged in.
  ding_user_get_creds($account);
  return array(
    'aby' => 'Andeby bibliotek',
    'grd' => 'Gåserød bibliotek',
  );
}

/**
 *
 */
function connie_reservation_set_preferred_branch($account, $branch) {
  // Throw exception if we're not logged in.
  ding_user_get_creds($account);
  // Do nothing, ding_reservation will update local user if we don't throw an
  // exception.
}

/**
 *
 */
function connie_reservation_list($account) {
  // Throw exception if we're not logged in.
  ding_user_get_creds($account);
  $result = array();
  if (isset($_SESSION['connie_reservations'])) {
    foreach ($_SESSION['connie_reservations'] as $id => $res) {
      $result[$id] = new DingProviderReservation($id, array(
        'ding_entity_id' => variable_get('ting_agency', '') . ':' . $res[0],
        'display_name' => $res[1],
        'pickup_branch_id' => $res[2],
        'expiry' => $res[3],
      ));
    }
  }
  // Fake a remote reservation.
  $result['ill-123'] = new DingProviderReservation('ill-123', array(
    'display_name' => 'Remote loan: The algebraist',
    'pickup_branch_id' => 'grd',
    'expiry' => date('Y-m-d', REQUEST_TIME + 86400),
    'ready_for_pickup' => TRUE,
  ));
  return $result;
}

/**
 * Provide options for reservation.
 */
function connie_reservation_options($type, $account, $reservables) {
  /*
   * While we use ding_reservation for branch handling, we don't want to
   * depend on the module, as it would mean that ding_reservation can't be
   * disabled. So instead we throw an exception here.
   */
  if (!module_exists('ding_reservation')) {
    throw new Exception('Openruth reservation options requires ding_reservation.');
  }
  $form = array();
  $profile = ding_user_provider_profile($account);
  $form += ding_reservation_default_options_branch($type, 'connie_preferred_branch', $profile->connie_preferred_branch[field_language('profile2', $profile, 'connie_preferred_branch')][0]['value'], connie__allowed_branches());

  return $form;
}

/**
 * Validate reservations options.
 */
function connie_reservation_options_validate($type, $account, $reservables, $values) {
  $result = array();
  return ding_reservation_default_options_branch_validate($type, 'openruth_preferred_branch', $profile_branch, $values);
}

/**
 * Submit handler for reservations options.
 */
function connie_reservation_options_submit($type, $account, $reservables, $values) {
  $profile = ding_user_provider_profile($account);
  $field_language = field_language('profile2', $profile);
  $update = ding_reservation_default_options_branch_submit($type, 'openruth_preferred_branch', $profile->connie_preferred_branch[$field_language['connie_preferred_branch']][0]['value'], $values);
  if (!empty($update['connie_preferred_branch'])) {
    $profile->connie_preferred_branch[$field_language['connie_preferred_branch']][0]['value'] = $update['connie_preferred_branch'];
    $profile->save();
  }
}

/**
 *
 */
function connie_reservation_create($account, $id, $options) {
  $branch = $options['connie_preferred_branch'];
  $expiry = REQUEST_TIME + 180;
  // Throw exception if we're not logged in.
  ding_user_get_creds($account);
  // Fake it.
  if (!isset($_SESSION['connie_reservations'])) {
    $_SESSION['connie_reservations'] = array();
  }
  foreach ($_SESSION['connie_reservations'] as $res) {
    if ($id == $res[0]) {
      //throw new DingProviderReservationExists();
    }
  }
  $_SESSION['connie_reservations'][uniqid()] = array($id, 'Reservation #' . (sizeof($_SESSION['connie_reservations']) + 1), $branch, date('Y-m-d', $expiry));
  return array('branch' => $branch);
}

/**
 *
 */
function connie_reservation_delete($account, $id) {
  // Throw exception if we're not logged in.
  ding_user_get_creds($account);
  if (isset($_SESSION['connie_reservations']) && isset($_SESSION['connie_reservations'][$id])) {
    unset($_SESSION['connie_reservations'][$id]);
  }
}

/**
 *
 * Should cache the result of external requests internally, as it
 * might be called multiple times with the same parameters in the same
 * request.
 */
function connie_reservation_branch_name($branch_id) {
  $branches = array(
    'aby' => 'Andeby bibliotek',
    'grd' => 'Gåserød bibliotek',
  );
  if (isset($branches[$branch_id])) {
    return $branches[$branch_id];
  }
  return NULL;
}
