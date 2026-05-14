import { test, expect } from '@playwright/test';

test.describe('Min Messenger E2E Tests', () => {
  test.beforeEach(async ({ page }) => {
    // Clear local storage before each test
    await page.goto('http://localhost:3000');
    await page.evaluate(() => localStorage.clear());
    await page.reload();
  });

  test('should load login page', async ({ page }) => {
    await page.goto('http://localhost:3000/login');
    await expect(page.locator('h2')).toHaveText('Login');
    await expect(page.locator('input[type="text"]')).toBeVisible();
    await expect(page.locator('input[type="password"]')).toBeVisible();
  });

  test('should navigate to register page', async ({ page }) => {
    await page.goto('http://localhost:3000/login');
    await page.click('text=Register');
    await expect(page.locator('h2')).toHaveText('Register');
  });

  test('should show validation errors on empty form', async ({ page }) => {
    await page.goto('http://localhost:3000/login');
    await page.click('button[type="submit"]');
    // Browser validation should prevent submission
  });

  test('should register a new user', async ({ page }) => {
    await page.goto('http://localhost:3000/register');
    
    await page.fill('input[type="text"]', 'testuser');
    await page.fill('input[placeholder="Type a message..."]').first().locator('..').locator('input').first();
    
    // Fill all fields
    const inputs = page.locator('input');
    await inputs.nth(0).fill('testuser'); // login
    await inputs.nth(1).fill('Test User'); // name
    await inputs.nth(2).fill('test@example.com'); // email
    await inputs.nth(3).fill('+1234567890'); // phone
    await inputs.nth(4).fill('password123'); // password
    
    await page.click('button[type="submit"]');
    
    // Should redirect to messages after success
    await expect(page).toHaveURL(/.*messages/, { timeout: 5000 });
  });

  test('should login with existing user', async ({ page }) => {
    // First register
    await page.goto('http://localhost:3000/register');
    const inputs = page.locator('input');
    await inputs.nth(0).fill('logintest');
    await inputs.nth(1).fill('Login Test');
    await inputs.nth(2).fill('login@example.com');
    await inputs.nth(3).fill('+0987654321');
    await inputs.nth(4).fill('password123');
    await page.click('button[type="submit"]');
    await page.waitForURL(/.*messages/, { timeout: 5000 });
    
    // Logout
    await page.click('text=Logout');
    await expect(page).toHaveURL(/.*login/, { timeout: 5000 });
    
    // Login again
    await page.fill('input[type="text"]', 'logintest');
    await page.fill('input[type="password"]', 'password123');
    await page.click('button[type="submit"]');
    await expect(page).toHaveURL(/.*messages/, { timeout: 5000 });
  });

  test('should show navbar when logged in', async ({ page }) => {
    await page.goto('http://localhost:3000/register');
    const inputs = page.locator('input');
    await inputs.nth(0).fill('navtest');
    await inputs.nth(1).fill('Nav Test');
    await inputs.nth(2).fill('nav@example.com');
    await inputs.nth(3).fill('+1111111111');
    await inputs.nth(4).fill('password123');
    await page.click('button[type="submit"]');
    await page.waitForURL(/.*messages/, { timeout: 5000 });
    
    // Check navbar links
    await expect(page.locator('text=Messages')).toBeVisible();
    await expect(page.locator('text=Notifications')).toBeVisible();
    await expect(page.locator('text=Files')).toBeVisible();
    await expect(page.locator('text=Reactions')).toBeVisible();
    await expect(page.locator('text=Status')).toBeVisible();
  });

  test('should navigate to Messages page', async ({ page }) => {
    await page.goto('http://localhost:3000/register');
    const inputs = page.locator('input');
    await inputs.nth(0).fill('msgtnav');
    await inputs.nth(1).fill('Msg Nav');
    await inputs.nth(2).fill('navmsg@example.com');
    await inputs.nth(3).fill('+2222222222');
    await inputs.nth(4).fill('password123');
    await page.click('button[type="submit"]');
    await page.waitForURL(/.*messages/, { timeout: 5000 });
    
    await page.click('text=Messages');
    await expect(page.locator('h2')).toHaveText('Messages');
    await expect(page.locator('select')).toBeVisible();
  });

  test('should navigate to Notifications page', async ({ page }) => {
    await page.goto('http://localhost:3000/register');
    const inputs = page.locator('input');
    await inputs.nth(0).fill('notiftest');
    await inputs.nth(1).fill('Notif Test');
    await inputs.nth(2).fill('notif@example.com');
    await inputs.nth(3).fill('+3333333333');
    await inputs.nth(4).fill('password123');
    await page.click('button[type="submit"]');
    await page.waitForURL(/.*messages/, { timeout: 5000 });
    
    await page.click('text=Notifications');
    await expect(page.locator('h2')).toHaveText('Notifications');
  });

  test('should navigate to Files page', async ({ page }) => {
    await page.goto('http://localhost:3000/register');
    const inputs = page.locator('input');
    await inputs.nth(0).fill('filetest');
    await inputs.nth(1).fill('File Test');
    await inputs.nth(2).fill('file@example.com');
    await inputs.nth(3).fill('+4444444444');
    await inputs.nth(4).fill('password123');
    await page.click('button[type="submit"]');
    await page.waitForURL(/.*messages/, { timeout: 5000 });
    
    await page.click('text=Files');
    await expect(page.locator('h2')).toHaveText('Files');
  });

  test('should navigate to Reactions page', async ({ page }) => {
    await page.goto('http://localhost:3000/register');
    const inputs = page.locator('input');
    await inputs.nth(0).fill('reacttest');
    await inputs.nth(1).fill('React Test');
    await inputs.nth(2).fill('react@example.com');
    await inputs.nth(3).fill('+5555555555');
    await inputs.nth(4).fill('password123');
    await page.click('button[type="submit"]');
    await page.waitForURL(/.*messages/, { timeout: 5000 });
    
    await page.click('text=Reactions');
    await expect(page.locator('h2')).toHaveText('Reactions');
  });

  test('should navigate to Status page', async ({ page }) => {
    await page.goto('http://localhost:3000/register');
    const inputs = page.locator('input');
    await inputs.nth(0).fill('statustest');
    await inputs.nth(1).fill('Status Test');
    await inputs.nth(2).fill('status@example.com');
    await inputs.nth(3).fill('+6666666666');
    await inputs.nth(4).fill('password123');
    await page.click('button[type="submit"]');
    await page.waitForURL(/.*messages/, { timeout: 5000 });
    
    await page.click('text=Status');
    await expect(page.locator('h2')).toHaveText('Status');
  });

  test('should redirect to login when accessing protected route', async ({ page }) => {
    await page.goto('http://localhost:3000/messages');
    // Should redirect to login
    await expect(page).toHaveURL(/.*login/, { timeout: 5000 });
  });

  test('should logout and clear local storage', async ({ page }) => {
    // Login first
    await page.goto('http://localhost:3000/register');
    const inputs = page.locator('input');
    await inputs.nth(0).fill('logouttest');
    await inputs.nth(1).fill('Logout Test');
    await inputs.nth(2).fill('logout@example.com');
    await inputs.nth(3).fill('+7777777777');
    await inputs.nth(4).fill('password123');
    await page.click('button[type="submit"]');
    await page.waitForURL(/.*messages/, { timeout: 5000 });
    
    // Verify local storage has token
    const token = await page.evaluate(() => localStorage.getItem('token'));
    expect(token).toBeTruthy();
    
    // Logout
    await page.click('text=Logout');
    
    // Token should be cleared
    const tokenAfterLogout = await page.evaluate(() => localStorage.getItem('token'));
    expect(tokenAfterLogout).toBeNull();
  });
});
